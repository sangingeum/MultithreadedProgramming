#pragma once
#include <functional>
#include <mutex>
#include <condition_variable>

/* Note:
#include <barrier>
Use std::barrier instead of this implementation.
*/


// Barrier class implemented with a mutex and a condition variable
// Do not set m_expectedCount to 0. 
class Barrier
{
	unsigned m_expectedCount;
	unsigned m_currentCount{ 0 };
	unsigned m_stop{ 0 };
	std::function<void()> m_completionFunction;
	std::mutex m_mutex;
	std::condition_variable m_cond;
public:
	Barrier(unsigned count, std::function<void()> completionFunction)
		: m_expectedCount(count), m_completionFunction(completionFunction) {}
	void arriveAndWait() {
		std::unique_lock lock{m_mutex};
		m_currentCount++;
		if (m_currentCount == m_expectedCount) {
			cleanUp();
			lock.unlock();
			m_cond.notify_all();
		}
		else {
			unsigned stop = m_stop;
			m_cond.wait(lock, [&]() { return (stop != m_stop); });
		}
	}
	void drop() {
		std::unique_lock lock{m_mutex};
		m_expectedCount--;
		if (m_currentCount == m_expectedCount) {
			cleanUp();
			lock.unlock();
			m_cond.notify_all();
		}
	}
	void arrive() {
		std::unique_lock lock{m_mutex};
		m_currentCount++;
		if (m_currentCount == m_expectedCount) {
			cleanUp();
			lock.unlock();
			m_cond.notify_all();
		}
	}
	void wait() {
		std::unique_lock lock{m_mutex};
		unsigned stop = m_stop;
		m_cond.wait(lock, [&]() { return (stop != m_stop); });
	}

private:
	inline void cleanUp() {
		m_completionFunction();
		m_currentCount = 0;
		m_stop++;
	}

};

