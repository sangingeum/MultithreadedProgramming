#pragma once
#include <atomic>

// TicketLock implemented with atomic variables
// unlock() should not be called from threads that don't hold the lock
// Busy Waiting
class TicketLock
{
	std::atomic<unsigned> ticket{0};
	std::atomic<unsigned> turn{0};
public:
	void lock() {
		// Receive a ticket number and wait for its turn
		unsigned myTicket = ticket.fetch_add(1, std::memory_order_relaxed);
		while(!(turn.load(std::memory_order_acquire) == myTicket)){} // Consider adding std::this_thread::yield(); inside the loop if tasks have long lifetimes
	}
	void unlock() {
		turn.fetch_add(1, std::memory_order_release);
	}
};

