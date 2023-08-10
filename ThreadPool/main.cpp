#include "ThreadPool.hpp"
#include <vector>
#include <iostream>
#include <string>

void sayHi() {
	std::cout << "Hi\n";
}

void saySomething(const std::string& something) {
	std::cout << something + "\n";
}

std::vector<int> accumulate(const std::vector<int>& vec) {
	auto result = vec;
	for (size_t i = 1; i < result.size(); ++i)
		result[i] = std::move(result[i]) + result[i - 1];
	return result;
}

int five() {
	return 5;
}

int main() {
	// Create a thread pool
	ThreadPool pool;
	// Containers for futures
	std::vector<std::future<void>> voidFutures;
	std::vector<std::future<std::vector<int>>> vecFutures;
	std::vector<std::future<int>> intFutures;

	// Submit tasks to the pool
	// std::function, std::bind, lambdas, and function names can be passed as an argument to the submit method
	std::vector<int> vec {1, 3, 5, 7};
	for (size_t i = 0; i < 10; ++i) {
		voidFutures.emplace_back(pool.submit(sayHi));
		voidFutures.emplace_back(pool.submit([]() {saySomething("Something"); }));
		vecFutures.emplace_back(pool.submit(std::bind(accumulate, std::cref(vec))));
		intFutures.emplace_back(pool.submit(five));
	}

	// Join and print the results
	for (auto& future : voidFutures) {
		future.get();
	}
	for (auto& future : vecFutures) {
		auto vec = future.get();
		for (auto num : vec)
			std::cout << num << " ";
		std::cout << "\n";
	}
	for (auto& future : intFutures) {
		std::cout << future.get() << "\n";
	}

	/* Possible result:
	Hi
	Something
	Something
	Hi
	Something
	Something
	Something
	Something
	Hi
	Hi
	Hi
	Something
	Hi
	Something
	Hi
	Something
	Hi
	Something
	Hi
	Hi
	1 4 9 16
	1 4 9 16
	1 4 9 16
	1 4 9 16
	1 4 9 16
	1 4 9 16
	1 4 9 16
	1 4 9 16
	1 4 9 16
	1 4 9 16
	5
	5
	5
	5
	5
	5
	5
	5
	5
	5
	*/

	return 0;
}