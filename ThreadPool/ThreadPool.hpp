#pragma once
#include <thread>
#include <future>
#include <functional>
#include <type_traits>
#include <memory>
#include "TSQueue.hpp"

class ThreadPool
{	
	bool m_done{ false };
	size_t m_numThreads;
	TSQueue<std::function<void()>> m_queue;
	std::vector<std::jthread> m_threads;
public:
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	ThreadPool(size_t numThreads = std::thread::hardware_concurrency())
		: m_numThreads(numThreads)
	{	
		try
		{
			for (size_t i = 0; i < m_numThreads; ++i) {
				m_threads.emplace_back(&ThreadPool::work, this);
			}
		}
		catch (const std::exception&)
		{
			m_done = true;
			throw;
		}
	}
	~ThreadPool() {
		m_done = true;
	}

	template <class Func>
	std::future<typename std::invoke_result<Func>::type> submit(Func func) {
		std::packaged_task task{std::move(func)};
		auto future{task.get_future()};
		// Wrap the packaged task with a lambda function to store it in the queue
		m_queue.push([t = std::make_shared<decltype(task)>(std::move(task))]() { (*t)(); });
		return future;
	}


private:
	void work() {
		while (!m_done) {
			std::function<void()> task;
			if (m_queue.tryPop(task))
				task();
			else
				std::this_thread::yield();
		}
	}

};

