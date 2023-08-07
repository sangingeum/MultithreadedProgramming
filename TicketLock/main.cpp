#include "TicketLock.hpp"
#include <thread>
#include <vector>
#include <iostream>
#include <latch>

TicketLock lock;
constexpr size_t numThreads{ 10 };
size_t counter{ 0 };
std::latch latch{numThreads};

void increment() {
	latch.arrive_and_wait(); // Make the threads start at the same time
	for (size_t i = 0; i < 1000; ++i) {
		lock.lock();
		counter++;
		lock.unlock();
	}
}

int main() {
	// Launch 10 threads and join them
	std::vector<std::jthread> threads;
	for (size_t i = 0; i < numThreads; ++i)
		threads.emplace_back(increment);
	for (size_t i = 0; i < numThreads; ++i)
		threads[i].join();

	std::cout << counter << "\n"; // 10000

	return 0;
}