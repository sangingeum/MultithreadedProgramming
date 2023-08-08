#pragma once
#include <mutex>
#include <condition_variable>

/*Note:
In practice, use std::latch in the <latch> header instead of this implementation.
*/

// Latch synchronization primitive implemented with a mutex and a condition variable
class Latch
{
	size_t m_count;
	bool m_reachedZero{ false };
	std::mutex m_mutex;
	std::condition_variable m_cond;
public:
	Latch(size_t count) : m_count(count) { if (count == 0) m_reachedZero = true; }
	// Decrease the counter by 1 and wait for it to be 0
	void arriveAndWait() {
		if (!m_reachedZero) {
			std::unique_lock lock{m_mutex};
			m_count--;
			if (m_count == 0) {
				m_reachedZero = true;
				lock.unlock();
				m_cond.notify_all();
			}
			else {
				m_cond.wait(lock, [&]() {return m_reachedZero; });
			}
		}
	}
	// Decrease the counter by 1
	void countDown() {
		if (!m_reachedZero) {
			std::unique_lock lock{m_mutex};
			m_count--;
			if (m_count == 0) {
				m_reachedZero = true;
				lock.unlock();
				m_cond.notify_all();
			}
		}
	}
	// Wait without decreasing the counter
	void wait() {
		if (!m_reachedZero) {
			std::unique_lock lock{m_mutex};
			m_cond.wait(lock, [&]() {return m_reachedZero; });
		}
	}

};

