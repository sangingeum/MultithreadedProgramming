#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>

template<class T>
class TSDeque
{	
private:
	std::deque<std::shared_ptr<T>> m_data;
	std::mutex m_mutex;
	std::condition_variable m_cond;
public:
	TSDeque(const TSDeque&) = delete;
	TSDeque& operator=(const TSDeque&) = delete;
	TSDeque() = default;
	void push(T item);
	bool tryPop(T& result);
	std::shared_ptr<T> tryPop();
	void waitAndPop(T& result);
	std::shared_ptr<T> waitAndPop();
	bool tryPopBack(T& result);
	std::shared_ptr<T> tryPopBack();
	void waitAndPopBack(T& result);
	std::shared_ptr<T> waitAndPopBack();
	bool empty() const;
};

template<class T>
void TSDeque<T>::push(T item) {
	auto itemPtr = std::make_shared<T>(std::move(item));
	std::unique_lock lock{m_mutex};
	m_data.emplace_back(itemPtr);
	lock.unlock();
	m_cond.notify_one();
}

template<class T>
bool TSDeque<T>::tryPop(T& result) {
	std::scoped_lock lock{m_mutex};
	if (m_data.empty()) return false;
	result = std::move(*m_data.front());
	m_data.pop_front();
	return true;
}

template<class T>
std::shared_ptr<T> TSDeque<T>::tryPop() {
	std::scoped_lock lock{m_mutex};
	if (m_data.empty()) return std::make_shared<T>();
	auto result = std::move(m_data.front());
	m_data.pop_front();
	return result;
}

template<class T>
void TSDeque<T>::waitAndPop(T& result) {
	std::unique_lock lock{m_mutex};
	m_cond.wait(lock, [&]() {return !m_data.empty(); });
	result = std::move(*m_data.front());
	m_data.pop_front();
}

template<class T>
std::shared_ptr<T> TSDeque<T>::waitAndPop() {
	std::unique_lock lock{m_mutex};
	m_cond.wait(lock, [this]() {return !m_data.empty(); });
	auto result = std::move(m_data.front());
	m_data.pop_front();
	return result;
}

template<class T>
bool TSDeque<T>::tryPopBack(T& result) {
	std::scoped_lock lock{m_mutex};
	if (m_data.empty()) return false;
	result = std::move(*m_data.back());
	m_data.pop_back();
	return true;
}

template<class T>
std::shared_ptr<T> TSDeque<T>::tryPopBack() {
	std::scoped_lock lock{m_mutex};
	if (m_data.empty()) return std::make_shared<T>();
	auto result = std::move(m_data.back());
	m_data.pop_back();
	return result;
}

template<class T>
void TSDeque<T>::waitAndPopBack(T& result) {
	std::unique_lock lock{m_mutex};
	m_cond.wait(lock, [&]() {return !m_data.empty(); });
	result = std::move(*m_data.back());
	m_data.pop_back();
}

template<class T>
std::shared_ptr<T> TSDeque<T>::waitAndPopBack() {
	std::unique_lock lock{m_mutex};
	m_cond.wait(lock, [this]() {return !m_data.empty(); });
	auto result = std::move(m_data.back());
	m_data.pop_back();
	return result;
}

template<class T>
bool TSDeque<T>::empty() const {
	std::scoped_lock lock{m_mutex};
	return m_data.empty();
}