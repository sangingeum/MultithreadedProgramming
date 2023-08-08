#include "Semaphore.hpp"
#include <thread>
#include <vector>
#include <iostream>
#include <format>
#include <latch>
#include <atomic>

Semaphore sema{ 4 }; // 4 threads can enter the critical area
std::latch latch{ 10 };
std::atomic<unsigned> counter{ 0 }; // Thread counter

void testSema() {
	latch.arrive_and_wait(); // Make sure the threads start at the same time
	sema.acquire();
	// Count the number of threads in the loop
	counter++;
	std::cout << std::format("{} threads are in the loop\n", counter.load());
	// Long task
	for (size_t i = 0; i < 100000; ++i) {
		int x = 1000;
		int y = x * x;
	}
	counter--;
	sema.release();
}

int main() {

	// Launch 10 threads that executes the testSema function
	std::vector<std::jthread> threads;
	for (size_t i = 0; i < 10; ++i) {
		threads.emplace_back(testSema);
	}

	/* Possible result:
	2 threads are in the loop
	3 threads are in the loop
	4 threads are in the loop
	1 threads are in the loop
	4 threads are in the loop
	4 threads are in the loop
	4 threads are in the loop
	4 threads are in the loop
	4 threads are in the loop
	4 threads are in the loop
	*/

	return 0;
}