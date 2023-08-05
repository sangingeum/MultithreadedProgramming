#include <iostream>
#include <functional>
#include <thread>
#include <vector>

void functionPointer() {
	std::cout << "function pointer\n";
}

class functionObject {
	std::vector<int> vec{1, 2, 3, 4};
public:
	void operator()() const{
		std::cout << "function object\n";
		for (const auto& num : vec) std::cout << num << " ";
		std::cout << "\n";
	}
};

class SomeClass {
public:
	void memberfunction() {
		std::cout << "member function\n";
	}
};

int main() {
	// 4 different ways to initialize a std::thread object

	// Function pointer
	std::thread t1{functionPointer};

	// Lambda function
	std::thread t2{[]() {std::cout << "lambda function\n"; }};

	// Function object
	functionObject funcObj;
	// * Copy
	std::thread t3{funcObj};
	// * Reference
	std::thread t4{std::ref(funcObj)};

	// Member function
	SomeClass instance;
	std::thread t5{&SomeClass::memberfunction, &instance};

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();

	return 0;
}