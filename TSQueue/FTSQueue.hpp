#pragma once
#include <mutex>
#include <memory>

// Fine-graind lock-based thread-safe queue
template<class T>
class FTSQueue
{
private:
	struct Node {
		std::shared_ptr<T> data;
		std::unique_ptr<Node> next{ nullptr };
	};
	std::unique_ptr<Node> head;
	Node* tail;
	std::mutex m_headMutex;
	std::mutex m_tailMutex;
public:
	FTSQueue(const FTSQueue&) = delete;
	FTSQueue& operator=(const FTSQueue&) = delete;
	FTSQueue() : head(new Node()), tail(head.get()) {}
	void push(T item);
	bool tryPop(T& result);
	std::shared_ptr<T> tryPop();
private:
	Node* getTail() {
		std::scoped_lock lock{ m_tailMutex };
		return tail;
	}
};

template<class T>
void FTSQueue<T>::push(T item) {
	auto newData = std::make_shared<T>(std::move(item));
	auto newNode = std::unique_ptr<Node>(new Node());
	std::scoped_lock lock{ m_tailMutex };
	Node* newDummyPtr = newNode.get();
	tail->data = newData;
	tail->next = std::move(newNode);
	tail = newDummyPtr;
}

template<class T>
bool FTSQueue<T>::tryPop(T& result) {
	std::scoped_lock lock{ m_headMutex };
	if (head.get() == getTail())
		return false;
	auto oldHead = std::move(head);
	result = std::move(*(oldHead->data));
	head = std::move(oldHead->next);
	return true;
}

template<class T>
std::shared_ptr<T> FTSQueue<T>::tryPop() {
	std::scoped_lock lock{ m_headMutex };
	if (head.get() == getTail())
		return std::shared_ptr<T>();
	auto oldHead = std::move(head);
	head = std::move(oldHead->next);
	return oldHead->data;
}
