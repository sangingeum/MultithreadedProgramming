#include "LFStack.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <unordered_map>
#include <cassert>
#include <latch>

constexpr size_t iter {2};
std::latch latch{iter * 3}; // Make sure the threads start at the same time

void push(LFStack<int>& stack) {
	latch.arrive_and_wait();
	for (size_t i = 0; i < 10000; ++i) {
		stack.push(i);
	}
}

std::vector<int> tryPopRef(LFStack<int>& stack) {
	std::vector<int> result;
	latch.arrive_and_wait();
	for (size_t i = 0; i < 5000; ++i) {
		int item;
		while (!stack.tryPop(item)) {}
		result.push_back(item);
	}
	return result;
}

std::vector<int> tryPopPtr(LFStack<int>& stack) {
	std::vector<int> result;
	latch.arrive_and_wait();
	for (size_t i = 0; i < 5000; ++i) {
		int item;
		std::shared_ptr<int> data;
		while (!(data = stack.tryPop())) {}
		result.push_back(std::move(*data));
	}
	return result;
}


int main() {
	// Create a thread-safe stack and containers for futures and threads
	LFStack<int> stack;
	std::vector<std::future<std::vector<int>>> futures;
	std::vector<std::jthread> threads;

	// Launch threads that push 10000 integers to the stack 
	for (size_t i = 0; i < iter; ++i) {
		threads.emplace_back(push, std::ref(stack));
	}

	// Launch threads that pops 5000 pushed integers using the tryPopRef function
	for (size_t i = 0; i < iter; ++i) {
		std::packaged_task<std::vector<int>(LFStack<int>&)> popTask{tryPopRef};
		futures.emplace_back(popTask.get_future());
		threads.emplace_back(std::move(popTask), std::ref(stack));
	}

	// Launch threads that pops 5000 pushed integers using the tryPopPtr function
	for (size_t i = 0; i < iter; ++i) {
		std::packaged_task<std::vector<int>(LFStack<int>&)> popTask{tryPopPtr};
		futures.emplace_back(popTask.get_future());
		threads.emplace_back(std::move(popTask), std::ref(stack));
	}

	// Count the occurrences of each number
	std::unordered_map<int, int> dist;
	for (size_t i = 0; i < futures.size(); ++i) {
		for (const auto& item : futures[i].get()) {
			++dist[item];
		}
	}

	// Check if the counts are 'iter'
	for (const auto& pair : dist) {
		assert(pair.second == iter);
	}

	return 0;
}