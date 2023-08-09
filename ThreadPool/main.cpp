#include "ThreadPool.hpp"
#include <vector>
#include <iostream>

void hi() {
	std::cout << "Hi\n";
}

int main() {
	ThreadPool pool(1);
	std::vector<std::future<void>> futures;
	for (size_t i = 0; i < 100; ++i) {
		futures.emplace_back(pool.submit(hi));
	}
	for (auto& future : futures) {
		future.get();
	}

	return 0;
}