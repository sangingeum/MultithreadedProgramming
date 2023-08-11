#include "WSThreadPool.hpp"


// Constructor: Initialize the thread pool with a specified number of threads
	// Leave 2 cores unused for other applications or the OS
WSThreadPool::WSThreadPool(size_t numThreads)
	: m_numThreads(numThreads)
{
	m_queues.reserve(numThreads);
	for (size_t i = 0; i < numThreads; ++i)
		m_queues.emplace_back(std::unique_ptr<WorkQueue>(new WorkQueue()));
	m_threads.reserve(numThreads);
	try
	{
		for (size_t i = 0; i < numThreads; ++i)
			m_threads.emplace_back(std::bind(&WSThreadPool::work, this, i, std::placeholders::_1));
	}
	catch (const std::exception&)
	{
		stopAllThreads();
		throw;
	}
}
// Run a pending task if any
void WSThreadPool::runPendingTask() {
	std::function<void()> task;
	if (getWork(task))
		task();
	else
		std::this_thread::yield();
}

// Destructor: Stop all threads in the pool
WSThreadPool::~WSThreadPool() {
	stopAllThreads();
}


// Worker function for each thread
void WSThreadPool::work(size_t threadIndex, std::stop_token token) {
	m_threadIndex = threadIndex;
	m_localQueue = m_queues[threadIndex].get();
	while (!token.stop_requested()) {
		std::function<void()> task;
		if (getWork(task))
			task();
		else
			std::this_thread::yield();
	}
}

// Take or steal an available task
bool WSThreadPool::getWork(std::function<void()>& task) {
	// Search the local queue
	if (m_localQueue && m_localQueue->tryPop(task))
		return true;
	// Search the main queue
	if (m_mainQueue.tryPop(task))
		return true;
	// Steal a work from other queues
	for (size_t i = 1; i < m_numThreads; ++i) {
		size_t nextIndex = (m_threadIndex + i) % m_numThreads;
		if (m_queues[nextIndex]->tryPopBack(task))
			return true;
	}
	return false;
}

// Stop all threads in the pool
void WSThreadPool::stopAllThreads() {
	for (auto& thread : m_threads)
		thread.request_stop();
}