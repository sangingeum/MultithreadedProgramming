#pragma once
#include <queue>
#include <mutex>
#include <memory>
template <class T>
class TSQueue
{
private:
	std::queue<std::shared_ptr<T>> m_data;
	mutable std::mutex m_mutex;
	std::condition_variable m_cond;
public:
	TSQueue() = default;
	TSQueue(const TSQueue& other) = delete;
	TSQueue& operator=(const TSQueue& other) = delete;
	void push(T item);
	bool tryPop(T& result);
	std::shared_ptr<T> tryPop();
	void waitAndPop(T& result);
	std::shared_ptr<T> waitAndPop();
	bool empty() const;
};


template <class T>
void TSQueue<T>::push(T item) {
	auto itemPtr = std::make_shared<T>(std::move(item));
	std::scoped_lock lock{m_mutex};
	m_data.push(itemPtr);
	m_cond.notify_one();
}

template <class T>
bool TSQueue<T>::tryPop(T& result) {
	std::scoped_lock lock{m_mutex};
	if (m_data.empty()) return false;
	result = std::move(*m_data.front());
	m_data.pop();
	return true;
}

template <class T>
std::shared_ptr<T> TSQueue<T>::tryPop() {
	std::scoped_lock lock{m_mutex};
	if (m_data.empty()) return std::make_shared<T>();
	auto result = std::move(m_data.front());
	m_data.pop();
	return result;
}

template <class T>
void TSQueue<T>::waitAndPop(T& result) {
	std::unique_lock lock{m_mutex};
	m_cond.wait(lock, [this]() {return !m_data.empty(); });
	result = std::move(*m_data.front());
	m_data.pop();
}

template <class T>
std::shared_ptr<T> TSQueue<T>::waitAndPop() {
	std::unique_lock lock{m_mutex};
	m_cond.wait(lock, [this]() {return !m_data.empty(); });
	auto result = std::move(m_data.front());
	m_data.pop();
	return result;
}

template <class T>
bool TSQueue<T>::empty() const {
	std::scoped_lock lock{m_mutex};
	return m_data.empty();
}