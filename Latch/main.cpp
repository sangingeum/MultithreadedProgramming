#include "Latch.hpp"
#include <thread>
#include <vector>
#include <iostream>
#include <format>

constexpr size_t numThreads{5};
Latch latch{5};

void testLatch() {
	// Check if the 'Before latch' string is always 
	// before the 'After latch' string in the console
	std::cout << std::format("Before latch\n");
	latch.arriveAndWait();
	std::cout << std::format("After latch\n");
}

int main() {
	// Launch 5 threads that executes the testLatch function
	std::vector<std::jthread> threads;
	for (size_t i = 0; i < numThreads; ++i)
		threads.emplace_back(testLatch);
	
	/*
	Before latch
	Before latch
	Before latch
	Before latch
	Before latch
	After latch
	After latch
	After latch
	After latch
	After latch
	*/
	return 0;
}