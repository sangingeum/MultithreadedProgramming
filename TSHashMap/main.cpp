#include "TSHashMap.hpp"
#include <string>
#include <iostream>
#include <thread>
#include <future>
#include <latch>
#include <random>

std::latch latch{6};

void update(TSHashMap<size_t, float>& map, const std::vector<size_t>& ids) {
	std::srand(std::time(nullptr));
	latch.arrive_and_wait();
	for (auto id : ids) {
		map.addOrUpdate(id, std::rand()*0.01f);
	}
}
void read(TSHashMap<size_t, float>& map, const std::vector<size_t>& ids) {
	latch.arrive_and_wait();
	for (auto id : ids) {
		map.get(id);
	}
}
void snapShot(TSHashMap<size_t, float>& map, const std::vector<size_t>& ids) {
	latch.arrive_and_wait();
	for (size_t i = 0; i < 10; ++i) {
		map.snapShot();
	}
}

std::vector<size_t> init(TSHashMap<size_t, float>& map) {
	std::vector<size_t> ids;
	std::srand(std::time(nullptr));
	for (size_t i = 0; i < 1000; ++i) {
		auto id = std::rand();
		ids.push_back(id);
		map.addOrUpdate(id, i * 0.1);
	}
	return ids;
}

int main() {
	TSHashMap<size_t, float> map(1087);
	std::vector<size_t> ids = init(map);
	std::vector<std::jthread> threads;

	for (size_t i = 0; i < 2; ++i)
		threads.emplace_back(snapShot, std::ref(map), std::cref(ids));
	for (size_t i = 0; i < 2; ++i)
		threads.emplace_back(update, std::ref(map), std::cref(ids));
	for (size_t i = 0; i < 2; ++i)
		threads.emplace_back(read, std::ref(map), std::cref(ids));


	return 0;
}