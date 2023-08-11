#include "ThreadPool.hpp"

// Constructor: Initialize the thread pool with a specified number of threads
ThreadPool::ThreadPool(size_t numThreads)
	: m_numThreads(numThreads)
{
	try
	{	
		m_threads.reserve(numThreads);
		for (size_t i = 0; i < m_numThreads; ++i) {
			m_threads.emplace_back(std::bind_front(&ThreadPool::work, this));
		}
	}
	catch (const std::exception&)
	{
		stopAllThreads();
		throw;
	}
}

// Destructor: Stop all threads in the pool
ThreadPool::~ThreadPool() {
	stopAllThreads();
}

// Run a pending task if any
void ThreadPool::runPendingTask() {
	std::function<void()> task;
	if (m_queue.tryPop(task))
		task();
	else
		std::this_thread::yield();
}

// Worker function for each thread
void ThreadPool::work(std::stop_token token) {
	while (!token.stop_requested()) {
		std::function<void()> task;
		if (m_queue.tryPop(task))
			task();
		else
			std::this_thread::yield();
	}
}

// Stops all threads in the pool
void ThreadPool::stopAllThreads() {
	for (auto& thread : m_threads)
		thread.request_stop();
}