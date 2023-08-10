#include "ThreadPool.hpp"


ThreadPool::ThreadPool(size_t numThreads)
	: m_numThreads(numThreads)
{
	try
	{
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
ThreadPool::~ThreadPool() {
	stopAllThreads();
}

void ThreadPool::work(std::stop_token token) {
	while (!token.stop_requested()) {
		std::function<void()> task;
		if (m_queue.tryPop(task))
			task();
		else
			std::this_thread::yield();
	}
}

void ThreadPool::stopAllThreads() {
	for (auto& thread : m_threads)
		thread.request_stop();
}