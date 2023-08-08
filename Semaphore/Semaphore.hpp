#pragma once
#include <mutex>
#include <condition_variable>
#include <atomic>

/* Note:
#include <semaphore>
In practice, use std::counting_semaphore or std::binary_semaphore instead
*/

// Semaphore implemented with a mutex and a condition variable
class Semaphore
{
	unsigned m_counter;
	std::mutex m_mutex;
	std::condition_variable m_cond;
public:
	// At most 'count' threads can acquire the lock
	Semaphore(unsigned count) : m_counter(count){}
	void acquire() {
		std::unique_lock lock{m_mutex};
		if (m_counter <= 0)
			m_cond.wait(lock, [&]() {return m_counter > 0; });
		m_counter--;
	}
	void release() {
		std::unique_lock lock{m_mutex};
		m_counter++;
		lock.unlock();
		m_cond.notify_one();
	}
};

