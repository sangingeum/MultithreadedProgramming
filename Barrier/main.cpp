#include "Barrier.hpp"
#include <iostream>
#include <format>
#include <thread>
#include <vector>

constexpr size_t numTask1{ 3 };
constexpr size_t numTask2{ 2 };

Barrier barrier{ numTask1 + numTask2, [](){std::cout << "Executing completion function...\n";}};

// Test the arriveAndWait method
void task1() {
	for (size_t i = 0; i < 5; ++i) {
		auto id = std::hash<std::thread::id>()(std::this_thread::get_id());
		std::cout << std::format("Thread {} executing task1\n", id);
		barrier.arriveAndWait();
	}
}

// Test the drop method
// It drops from the barrier at the end
void task2() {
	for (size_t i = 0; i < 3; ++i) {
		auto id = std::hash<std::thread::id>()(std::this_thread::get_id());
		std::cout << std::format("Thread {} executing task2\n", id);
		barrier.arriveAndWait();
	}
	barrier.drop();
}

int main() {
	// Launch threads that execute task1 and task2 functions
	std::vector<std::jthread> threads;
	for (size_t i = 0; i < numTask1; ++i)
		threads.emplace_back(task1);
	for (size_t i = 0; i < numTask2; ++i)
		threads.emplace_back(task2);

	/* Possible Result:
	Thread 924741211481182423 executing task1
	Thread 272332880629171459 executing task2
	Thread 17899448552799363026 executing task1
	Thread 17759823146843228942 executing task1
	Thread 7525477558264133055 executing task2
	Executing completion function...
	Thread 17759823146843228942 executing task1
	Thread 7525477558264133055 executing task2
	Thread 272332880629171459 executing task2
	Thread 17899448552799363026 executing task1
	Thread 924741211481182423 executing task1
	Executing completion function...
	Thread 17899448552799363026 executing task1
	Thread 924741211481182423 executing task1
	Thread 272332880629171459 executing task2
	Thread 17759823146843228942 executing task1
	Thread 7525477558264133055 executing task2
	Executing completion function...
	Thread 924741211481182423 executing task1
	Thread 17759823146843228942 executing task1
	Thread 17899448552799363026 executing task1
	Executing completion function...
	Thread 17899448552799363026 executing task1
	Thread 924741211481182423 executing task1
	Thread 17759823146843228942 executing task1
	Executing completion function...
	*/

	return 0;
}