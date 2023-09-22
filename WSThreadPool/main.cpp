#include "WSThreadPool.hpp"
#include <vector>
#include <iostream>
#include <string>
#include <format>
#include <random>
#include <algorithm>
#include <chrono>

// Create a thread pool
WSThreadPool pool(12);

// Median of Three partition
size_t partition(std::vector<int>& arr, size_t p, size_t r) {
	size_t mid = (p+r)/2;
	size_t medianIndex;
	if ((arr[p] < arr[mid] && arr[mid] < arr[r]) || (arr[r] < arr[mid] && arr[mid] < arr[p]))
		medianIndex = mid;
	else if ((arr[mid] < arr[p] && arr[p] < arr[r]) || (arr[r] < arr[p] && arr[p] < arr[mid]))
		medianIndex = p;
	else
		medianIndex = r;
	std::swap(arr[medianIndex], arr[r]);

	auto x = arr[r];
	size_t i = p;
	for (size_t j = p; j < r; ++j) {
		if (arr[j] < x) {
			std::swap(arr[i++], arr[j]);
		}
	}
	std::swap(arr[i], arr[r]);
	return i;
}

// Parallel sort
void sort(std::vector<int>& vec, size_t from, size_t to) {
	const static size_t chunkSize = 100000;
	if (from >= to || to >= vec.size()) // 'to >= vec.size()' is needed to check if 'to' is underflowed
		return;
	size_t mid = partition(vec, from, to);
	// Do small jobs on your own
	if (to - from < chunkSize) {
		sort(vec, mid + 1, to);
		sort(vec, from, mid - 1);
	}
	// Pass the smaller job to the pool
	else {
		std::future<void> future;
		if (mid - from < to - mid) {
			future = pool.submit(std::bind(sort, std::ref(vec), from, mid - 1));
			sort(vec, mid + 1, to);
		}
		else {
			future = pool.submit(std::bind(sort, std::ref(vec), mid + 1, to));
			sort(vec, from, mid - 1);
		}
		// Run other jobs in the pool if the future is not ready
		while (!WSThreadPool::isFutureReady(future))
			pool.runPendingTask();
		future.get();
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
	sort(randomVector, 0, randomVector.size() - 1);
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
	Time taken for parallel sorting: 589ms
	Time taken for std::sort: 3488ms
	*/
	
	return 0;
}