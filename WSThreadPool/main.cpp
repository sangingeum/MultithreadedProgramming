#include "WSThreadPool.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <format>
#include <random>
#include <algorithm>
#include <chrono>

// Create a thread pool
WSThreadPool pool;
std::vector<std::future<void>> futures;

void sort(std::vector<int>& vec, size_t from, size_t to) {
	if (from >= to || to >= vec.size())
		return;
	const static size_t chunkSize = 1500000;
	//std::cout << std::format("from{}, to{}\n", from, to);
	auto pivot = vec[to];
	auto begin = vec.begin();
	auto midIt = std::partition(begin + from, begin + to,
		[pivot](const int& item) { return item < pivot; });
	size_t mid = midIt - begin;
	//std::cout << std::format("from{}, to{}, mid{}\n", from, to, mid);
	std::swap(vec[to], vec[mid]);

	if (to - from < chunkSize) {
		sort(vec, mid + 1, to);
		sort(vec, from, mid - 1);
	}
	else {
		if (mid - from < chunkSize) {
			futures.emplace_back(pool.submit(std::bind(sort, std::ref(vec), from, mid - 1)));
			sort(vec, mid + 1, to);
		}
		else {
			if (to - mid < chunkSize) {
				futures.emplace_back(pool.submit(std::bind(sort, std::ref(vec), mid + 1, to)));
				sort(vec, from, mid - 1);
			}
			else {
				sort(vec, mid + 1, to);
				sort(vec, from, mid - 1);
			}
		}
	}
}

void parallelSort(std::vector<int>& vec) {
	sort(vec, 0, vec.size() - 1);
	for (auto& future : futures) {
		future.wait();
	}
}

int main() {
	// Make a random vector of length 10000000
	std::random_device rd;  
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(-100000, 100000); 
	std::vector<int> randomVector(10000000);
	std::generate(randomVector.begin(), randomVector.end(), [&]() {
		return distribution(gen);
		});
	// Make a copy of the vector
	auto copy = randomVector;
	// Sort the random vector using threads in the thread pool
	auto t1 = std::chrono::steady_clock::now();
	parallelSort(randomVector);
	// Sort the copied vector using std::sort
	auto t2 = std::chrono::steady_clock::now();
	std::sort(copy.begin(), copy.end());
	auto t3 = std::chrono::steady_clock::now();

	// Check if the results are the same
	std::cout << "Results are the same: " << std::boolalpha << (randomVector == copy) << "\n";
	
	// Check time taken for each sorting
	std::cout << "Time taken for parallel sorting: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms\n";
	std::cout << "Time taken for std::sort: " << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() << "ms\n";

	/*
	Results are the same: true
	Time taken for parallel sorting: 14594ms
	Time taken for std::sort: 3877ms
	*/

	return 0;
}