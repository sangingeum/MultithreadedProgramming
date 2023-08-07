#include "TSHashMap.hpp"
#include <iostream>
#include <format>
#include <thread>
#include <future>
#include <latch>
#include <random>

constexpr size_t numThread{ 2 }; // The number of threads for each test
constexpr size_t numIter{ 10000 };
std::latch latch{numThread * 4};

// Test the addOrUpdate method [exclusive lock]
void updateTest(TSHashMap<size_t, float>& map) {
	std::srand(std::time(nullptr));
	size_t added = 0;
	latch.arrive_and_wait();
	for (size_t i = 0; i < numIter; ++i) {
		if (map.addOrUpdate(std::rand(), std::rand() * 0.01f))
			added++;
	}
	std::cout << std::format("added {} entities, updated {} entities\n", added, numIter - added);
}

// Test the get method [shared lock]
void readTest(TSHashMap<size_t, float>& map) {
	std::srand(std::time(nullptr));
	size_t read = 0;
	latch.arrive_and_wait();
	for (size_t i = 0; i < numIter; ++i) {
		float value;
		if (map.get(std::rand(), value))
			read++;
	}
	std::cout << std::format("read {} entities\n", read);
}

// Test the snapShot method [shared lock]
void snapShotTest(TSHashMap<size_t, float>& map) {
	size_t snapShots = 0;
	latch.arrive_and_wait();
	for (size_t i = 0; i < 10; ++i) {
		map.snapShot();
		snapShots++;
	}
	std::cout << std::format("Took {} snapShots\n", snapShots);
}

// Test the remove method [exclusive lock]
void removeTest(TSHashMap<size_t, float>& map) {
	std::srand(3);
	size_t removed = 0;
	latch.arrive_and_wait();
	for (size_t i = 0; i < numIter; ++i) {
		if (map.remove(std::rand()))
			removed++;
	}
	std::cout << std::format("removed {} entities\n", removed);
}

int main() {
	// Create a hash map with 5099 buckets
	// The key type is size_t and value type is float
	TSHashMap<size_t, float> map(5099);

	// Create a vector to store threads
	std::vector<std::jthread> threads;
	// Lanch test threads
	for (size_t i = 0; i < numThread; ++i)
		threads.emplace_back(snapShotTest, std::ref(map));
	for (size_t i = 0; i < numThread; ++i)
		threads.emplace_back(updateTest, std::ref(map));
	for (size_t i = 0; i < numThread; ++i)
		threads.emplace_back(readTest, std::ref(map));
	for (size_t i = 0; i < numThread; ++i)
		threads.emplace_back(removeTest, std::ref(map));

	/* Possible result:
	removed 457 entities
	read 5301 entities
	added 8729 entities, updated 1271 entities
	read 5256 entities
	added 227 entities, updated 9773 entities
	removed 960 entities
	Took 10 snapShots
	Took 10 snapShots
	*/
	return 0;
}