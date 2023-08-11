#pragma once
#include <thread>
#include <future>
#include <functional>
#include <type_traits>
#include <vector>
#include <memory>
#include "TSDeque.hpp"

// This code should be inside the class, but I couldn't find a way to do so...
static thread_local TSDeque<std::function<void()>>* m_localQueue;
static thread_local size_t m_threadIndex;
//

// Work stealing thread pool with distrubuted queues for each thread and a main queue
class WSThreadPool
{
private:
	using WorkQueue = TSDeque<std::function<void()>>;
	size_t m_numThreads; // Number of threads in the thread pool
	WorkQueue m_mainQueue;
	std::vector<std::unique_ptr<WorkQueue>> m_queues;
	std::vector<std::jthread> m_threads; // Vector to store thread objects
public:
	// Delete copy constructor and copy assignment operator
	WSThreadPool(const WSThreadPool&) = delete;
	WSThreadPool& operator=(const WSThreadPool&) = delete;
	// Constructor: Initialize the thread pool with a specified number of threads
	// Leave 2 cores unused for other applications or the OS
	WSThreadPool(size_t numThreads = std::max(std::thread::hardware_concurrency() - 2, 1u)) 
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

	void runPendingTask() {
		std::function<void()> task;
		if (getWork(task))
			task();
		else
			std::this_thread::yield();
	}

	// Destructor: Stop all threads in the pool
	~WSThreadPool() {
		stopAllThreads();
	}
	// Submit a callable task to the thread pool and returns a future for the result
	template <class Func>
	std::future<typename std::invoke_result<Func>::type> submit(Func func);

	template <class T>
	static bool isFutureReady(std::future<T>& future) {
		return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}

private:
	// Worker function for each thread
	void work(size_t threadIndex, std::stop_token token) {
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

	bool getWork(std::function<void()>& task) {
		// Search local queue
		if (m_localQueue && m_localQueue->tryPop(task))
			return true;
		// Search main queue
		if (m_mainQueue.tryPop(task))
			return true;
		// Steal work from other queues
		for (size_t i = 1; i < m_numThreads; ++i) {
			size_t nextIndex = (m_threadIndex + i) % m_numThreads;
			if (m_queues[nextIndex]->tryPopBack(task))
				return true;
		}
		return false;
	}

	// Stop all threads in the pool
	void stopAllThreads() {
		for (auto& thread : m_threads)
			thread.request_stop();
	}

};



// Submits a callable task to the thread pool and returns a future for the result
template <class Func>
std::future<typename std::invoke_result<Func>::type> WSThreadPool::submit(Func func) {
	using ResultType = typename std::invoke_result<Func>::type;
	std::packaged_task<ResultType()> task{std::move(func)};
	auto future{ task.get_future() };
	// Wrap the packaged task with a lambda function to store it in the queue
	if(m_localQueue)
		m_localQueue->push([t = std::make_shared<decltype(task)>(std::move(task))]() { (*t)(); });
	else
		m_mainQueue.push([t = std::make_shared<decltype(task)>(std::move(task))]() { (*t)(); });
	return future;
}


