#pragma once
#include <mutex>
#include <stack>
#include <memory>
//1. Ensure that no thread observes a state where the invariants
// of the data structure have been broken by other threads
//2. Avoid race conditions inherent in the interface of the data structure

// Thread-safe Stack implemented with a mutex and a condition variable
template<class T>
class TSStack
{
private:
	std::stack<std::shared_ptr<T>> m_data;
	mutable std::mutex m_mutex;
	std::condition_variable m_cond;
public:
	TSStack() = default;
	TSStack(const TSStack& other) = delete;
	TSStack& operator=(const TSStack& other) = delete;
	void push(T item);
	bool tryPop(T& result);
	std::shared_ptr<T> tryPop();
	void waitAndPop(T& result);
	std::shared_ptr<T> waitAndPop();
	bool empty() const;
};


// Push and notify any waiting thread
template<class T>
void TSStack<T>::push(T item) {
	auto itemPtr = std::make_shared<T>(std::move(item));
	std::scoped_lock lock{m_mutex};
	m_data.push(itemPtr);
	m_cond.notify_one();
}

// Try to pop a pushed item
// If successful, return true. Otherwise, return false
template<class T>
bool TSStack<T>::tryPop(T& result) {
	std::scoped_lock lock{m_mutex};
	if (m_data.empty()) return false;
	result = std::move(*m_data.top());
	m_data.pop();
	return true;
}

// Try to pop a pushed item
// If successful, return a shared pointer to the popped item
// Otherwise, return an empty shared pointer
template<class T>
std::shared_ptr<T> TSStack<T>::tryPop() {
	std::scoped_lock lock{m_mutex};
	if (m_data.empty()) return std::make_shared<T>();
	auto result = std::move(m_data.top());
	m_data.pop();
	return result;
}

// Wait for a pushed item and then pop it
template<class T>
void TSStack<T>::waitAndPop(T& result) {
	std::unique_lock lock{m_mutex};
	m_cond.wait(lock, [this]() {return !m_data.empty(); });
	result = std::move(*m_data.top());
	m_data.pop();
}

// Wait for a pushed item and then pop it
template<class T>
std::shared_ptr<T> TSStack<T>::waitAndPop() {
	std::unique_lock lock{m_mutex};
	m_cond.wait(lock, [this]() {return !m_data.empty(); });
	auto result = std::move(m_data.top());
	m_data.pop();
	return result;
}

// Check if the stack is empty
template<class T>
bool TSStack<T>::empty() const {
	std::scoped_lock lock{m_mutex};
	return m_data.empty();
}

