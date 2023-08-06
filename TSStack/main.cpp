#include "TSStack.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <unordered_map>
#include <cassert>
#include <latch>

constexpr size_t iter {2};
std::latch latch{iter * 3}; // Let the threads start at the same time

void push(TSStack<int>& stack) {
	latch.arrive_and_wait();
	for (size_t i = 0; i < 1000; ++i) {
		stack.push(i);
	}
}

std::vector<int> waitAndPop(TSStack<int>& stack) {
	std::vector<int> result;
	latch.arrive_and_wait();
	for (size_t i = 0; i < 500; ++i) {
		int item;
		stack.waitAndPop(item);
		result.push_back(item);
	}
	return result;
}

std::vector<int> tryPop(TSStack<int>& stack) {
	std::vector<int> result;
	latch.arrive_and_wait();
	for (size_t i = 0; i < 500; ++i) {
		int item;
		while (!stack.tryPop(item)) {}
		result.push_back(item);
	}
	return result;
}


int main() {
	// Create a thread-safe stack and containers for futures and threads
	TSStack<int> stack;
	std::vector<std::future<std::vector<int>>> futures;
	std::vector<std::jthread> threads;

	// Launch threads that push 1000 integers to the stack 
	for (size_t i = 0; i < iter; ++i) {
		threads.emplace_back(push, std::ref(stack));
	}

	// Launch threads that pops 500 pushed integers using the waitAndPop function
	for (size_t i = 0; i < iter; ++i) {
		std::packaged_task<std::vector<int>(TSStack<int>&)> popTask{waitAndPop};
		futures.emplace_back(popTask.get_future());
		threads.emplace_back(std::move(popTask), std::ref(stack));
	}

	// Launch threads that pops 500 pushed integers using the tryPop function
	for (size_t i = 0; i < iter; ++i) {
		std::packaged_task<std::vector<int>(TSStack<int>&)> popTask{tryPop};
		futures.emplace_back(popTask.get_future());
		threads.emplace_back(std::move(popTask), std::ref(stack));
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