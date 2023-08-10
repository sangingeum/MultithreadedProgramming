#pragma once
#include <thread>
#include <future>
#include <functional>
#include <type_traits>
#include <memory>
#include "TSQueue.hpp"

// Thread pool with a centralized queue
// Potentially high contention on the queue
// Poor cache utilization - tasks frequently move between processors
class ThreadPool
{
private:
	size_t m_numThreads; // Number of threads in the thread pool
	TSQueue<std::function<void()>> m_queue; // Thread-safe queue to hold tasks
	std::vector<std::jthread> m_threads; // Vector to store thread objects
public:
	// Delete copy constructor and copy assignment operator
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	// Constructor: Initialize the thread pool with a specified number of threads
	// Leave 2 cores unused for other applications or the OS
	ThreadPool(size_t numThreads = std::max(std::thread::hardware_concurrency() - 2, 1u));
	// Destructor: Stop all threads in the pool
	~ThreadPool();
	// Submit a callable task to the thread pool and returns a future for the result
	template <class Func>
	std::future<typename std::invoke_result<Func>::type> submit(Func func);
private:
	// Worker function for each thread
	void work(std::stop_token token);
	// Stop all threads in the pool
	void stopAllThreads();
};

// Submits a callable task to the thread pool and returns a future for the result
template <class Func>
std::future<typename std::invoke_result<Func>::type> ThreadPool::submit(Func func) {
	using ResultType = typename std::invoke_result<Func>::type;
	std::packaged_task<ResultType()> task{std::move(func)};
	auto future{ task.get_future() };
	// Wrap the packaged task with a lambda function to store it in the queue
	m_queue.push([t = std::make_shared<decltype(task)>(std::move(task))]() { (*t)(); });
	return future;
}


