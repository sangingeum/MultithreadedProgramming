#pragma once
#include <atomic>

// SpinLock class implemented with std::atomic_flag 
// std::atomic_flag is guaranteed to be lock-free
// Do not use it in practice => Busy Waiting, Priority Inversion
class SpinLock
{
	std::atomic_flag m_flag = ATOMIC_FLAG_INIT; // Initially set to false
public:
	void lock() {
		while(m_flag.test_and_set(std::memory_order_acquire)){}
	}
	void unlock() {
		m_flag.clear(std::memory_order_release);
	}
};

/* Quote from https://en.cppreference.com/w/cpp/atomic/memory_order:
When the lock is released by thread A and acquired by thread B, 
everything that took place in the critical section (before the release) 
in the context of thread A has to be visible to thread B (after the acquire) 
which is executing the same critical section.
*/
