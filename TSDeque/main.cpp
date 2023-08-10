#include "TSDeque.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <unordered_map>
#include <cassert>
#include <latch>

constexpr size_t iter {2};
std::latch latch{iter * 5}; // Make sure the threads start at the same time

void push(TSDeque<int>& deque) {
	latch.arrive_and_wait();
	for (size_t i = 0; i < 10000; ++i) {
		deque.push(i);
	}
}

std::vector<int> waitAndPop(TSDeque<int>& deque) {
	std::vector<int> result;
	latch.arrive_and_wait();
	for (size_t i = 0; i < 2500; ++i) {
		int item;
		deque.waitAndPop(item);
		result.push_back(item);
	}
	return result;
}

std::vector<int> tryPop(TSDeque<int>& deque) {
	std::vector<int> result;
	latch.arrive_and_wait();
	for (size_t i = 0; i < 2500; ++i) {
		int item;
		while (!deque.tryPop(item)) {}
		result.push_back(item);
	}
	return result;
}

std::vector<int> waitAndPopBack(TSDeque<int>& deque) {
	std::vector<int> result;
	latch.arrive_and_wait();
	for (size_t i = 0; i < 2500; ++i) {
		int item;
		deque.waitAndPopBack(item);
		result.push_back(item);
	}
	return result;
}

std::vector<int> tryPopBack(TSDeque<int>& deque) {
	std::vector<int> result;
	latch.arrive_and_wait();
	for (size_t i = 0; i < 2500; ++i) {
		int item;
		while (!deque.tryPopBack(item)) {}
		result.push_back(item);
	}
	return result;
}


int main() {
	// Create a thread-safe deque and containers for futures and threads
	TSDeque<int> deque;
	std::vector<std::future<std::vector<int>>> futures;
	std::vector<std::jthread> threads;

	// Launch threads that push 10000 integers to the deque 
	for (size_t i = 0; i < iter; ++i) {
		threads.emplace_back(push, std::ref(deque));
	}

	// Launch threads that pops 2500 pushed integers using the waitAndPop function
	for (size_t i = 0; i < iter; ++i) {
		std::packaged_task<std::vector<int>(TSDeque<int>&)> popTask{waitAndPop};
		futures.emplace_back(popTask.get_future());
		threads.emplace_back(std::move(popTask), std::ref(deque));
	}

	// Launch threads that pops 2500 pushed integers using the tryPop function
	for (size_t i = 0; i < iter; ++i) {
		std::packaged_task<std::vector<int>(TSDeque<int>&)> popTask{tryPop};
		futures.emplace_back(popTask.get_future());
		threads.emplace_back(std::move(popTask), std::ref(deque));
	}

	// Launch threads that pops 2500 pushed integers using the waitAndPopBack function
	for (size_t i = 0; i < iter; ++i) {
		std::packaged_task<std::vector<int>(TSDeque<int>&)> popTask{waitAndPopBack};
		futures.emplace_back(popTask.get_future());
		threads.emplace_back(std::move(popTask), std::ref(deque));
	}

	// Launch threads that pops 2500 pushed integers using the tryPopBack function
	for (size_t i = 0; i < iter; ++i) {
		std::packaged_task<std::vector<int>(TSDeque<int>&)> popTask{tryPopBack};
		futures.emplace_back(popTask.get_future());
		threads.emplace_back(std::move(popTask), std::ref(deque));
	}


	// Count the occurrences of each number
	std::unordered_map<int, int> dist;
	for (size_t i = 0; i < futures.size(); ++i) {
		for (const auto& item : futures[i].get()) {
			dist[item]++;
		}
	}

	// Check if the counts are 'iter'
	for (const auto& pair : dist) {
		assert(pair.second == iter);
	}


	return 0;
}