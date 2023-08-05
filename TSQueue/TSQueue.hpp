#pragma once
#include <queue>
#include <mutex>
#include <memory>
template <class T>
class TSQueue
{
private:
	std::queue<T> m_data;
	std::mutex m_mutex;
	std::condition_variable m_cond;
public:
	TSQueue() = default;
	TSQueue(const TSQueue& other) {
		std::scoped_lock lock{other.m_mutex};
		m_data = other.m_data;
	}
	TSQueue& operator=(const TSQueue& other) = delete;
	void push(T item) {
		std::scoped_lock lock{m_mutex};
		m_data.push(item);
	}
	bool tryPop(T& result) {
		std::scoped_lock lock{m_mutex};
		if (m_data.empty()) return false;
		result = std::move(m_data.front());
		m_data.pop();
		return true;
	}
	std::shared_ptr<T> tryPop() {
		std::scoped_lock lock{m_mutex};
		if (m_data.empty()) return std::make_shared<T>(nullptr);
		auto result = std::make_shared<T>(std::move(m_data.front()));
		return result;
	}
	void waitAndPop(T& result) {
		std::unique_lock lock{m_mutex};
		m_cond.wait(lock, [this](){return !m_data.empty(); });
		result = std::move(m_data.front());
		m_data.pop();
	}
	std::shared_ptr<T> waitAndPop() {
		std::unique_lock lock{m_mutex};
		m_cond.wait(lock, [this]() {return !m_data.empty(); });
		auto result = std::make_shared<T>(std::move(m_data.front()));
		return result;
	}
	bool empty() const {
		return m_data.empty();
	}
};

