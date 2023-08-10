#pragma once
#include <thread>
#include <future>
#include <functional>
#include <type_traits>
#include <memory>
#include "TSQueue.hpp"

class ThreadPool
{	
	size_t m_numThreads;
	TSQueue<std::function<void()>> m_queue;
	std::vector<std::jthread> m_threads;
public:
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
	~ThreadPool();
	template <class Func>
	std::future<typename std::invoke_result<Func>::type> submit(Func func);
private:
	void work(std::stop_token token);
	void stopAllThreads();
};

template <class Func>
std::future<typename std::invoke_result<Func>::type> ThreadPool::submit(Func func) {
	using ResultType = typename std::invoke_result<Func>::type;
	std::packaged_task<ResultType()> task{std::move(func)};
	auto future{ task.get_future() };
	// Wrap the packaged task with a lambda function to store it in the queue
	m_queue.push([t = std::make_shared<decltype(task)>(std::move(task))]() { (*t)(); });
	return future;
}


