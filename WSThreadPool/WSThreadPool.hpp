#pragma once
#include <thread>
#include <future>
#include <functional>
#include <type_traits>
#include <vector>
#include <memory>
#include "FTSQueue.hpp"

// This code should be inside the class, but I couldn't find a way to do so...
static thread_local FTSQueue<std::function<void()>>* m_localQueue;
static thread_local size_t m_threadIndex;

// Work stealing thread pool with distrubuted queues for each thread and a main queue
class WSThreadPool
{
private:
	using WorkQueue = FTSQueue<std::function<void()>>;
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
	WSThreadPool(size_t numThreads = std::max(std::thread::hardware_concurrency() - 2, 1u));
	// Run a pending task if any
	void runPendingTask();
	// Destructor: Stop all threads in the pool
	~WSThreadPool();
	// Submit a callable task to the thread pool and return a future for the result
	template <class Func>
	std::future<typename std::invoke_result<Func>::type> submit(Func func);
	// Check if the given future is ready
	template <class T>
	static bool isFutureReady(std::future<T>& future);
private:
	// Worker function for each thread
	void work(size_t threadIndex, std::stop_token token);
	// Take or steal an available task
	bool getWork(std::function<void()>& task);
	// Stop all threads in the pool
	void stopAllThreads();
};

// Submits a callable task to the thread pool and returns a future for the result
template <class Func>
std::future<typename std::invoke_result<Func>::type> WSThreadPool::submit(Func func) {
	using ResultType = typename std::invoke_result<Func>::type;
	std::packaged_task<ResultType()> task{std::move(func)};
	auto future{ task.get_future() };
	// Wrap the packaged task with a lambda function to store it in the queue
	if (m_localQueue)
		m_localQueue->push([t = std::make_shared<decltype(task)>(std::move(task))]() { (*t)(); });
	else
		m_mainQueue.push([t = std::make_shared<decltype(task)>(std::move(task))]() { (*t)(); });
	return future;
}

// Check if the given future is ready
template <class T>
static bool WSThreadPool::isFutureReady(std::future<T>& future) {
	return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}
