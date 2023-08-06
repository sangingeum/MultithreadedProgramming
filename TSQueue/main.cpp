#include "TSQueue.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <unordered_map>
#include <cassert>


void push(TSQueue<int>& queue) {
	for (size_t i = 0; i < 1000; ++i) {
		queue.push(i);
	}
}

std::vector<int> pop(TSQueue<int>& queue) {
	std::vector<int> result;
	for (size_t i = 0; i < 1000; ++i) {
		int item;
		queue.waitAndPop(item);
		result.push_back(item);
	}
	return result;
}

int main() {
	// Create a thread-safe queue and containers for futures and threads
	TSQueue<int> queue;
	std::vector<std::future<std::vector<int>>> futures;
	std::vector<std::jthread> threads;


	// Launch threads that push integers to the queue 
	for (size_t i = 0; i < 3; ++i) {
		threads.emplace_back(push, std::ref(queue));
	}

	// Launch threads that pops pushed intergers
	for (size_t i = 0; i < 3; ++i) {
		std::packaged_task<std::vector<int>(TSQueue<int>&)> popTask{pop};
		futures.emplace_back(popTask.get_future());
		threads.emplace_back(std::move(popTask), std::ref(queue));
	}


	// Count the occurrences of each number
	std::unordered_map<int, int> dist;
	for (size_t i = 0; i < 3; ++i) {
		for (const auto& item : futures[i].get()) {
			dist[item]++;
		}
	}

	// Check if the counts are 3
	for (const auto& pair : dist) {
		assert(pair.second == 3);
	}


	return 0;
}