#pragma once
#include <atomic>

// Ticket lock implemented with atomic variables
// Do not use it in practice. Use std::mutex instead.
// unlock() should not be called from threads that doesn't hold the lock
class TicketLock
{
	std::atomic<unsigned> ticket{0};
	std::atomic<unsigned> turn{0};
public:
	void lock() {
		// Receive a ticket number and wait for its turn
		unsigned myTicket = ticket.fetch_add(1, std::memory_order_relaxed);
		while(!(turn.load(std::memory_order_acquire) == myTicket)){}
	}
	void unlock() {
		turn.fetch_add(1, std::memory_order_release);
	}
};

