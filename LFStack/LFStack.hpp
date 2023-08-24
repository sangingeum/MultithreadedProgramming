#pragma once
#include <memory>
#include <atomic>

// Lock-free Thread-safe Stack implemented with atomic variables
template <class T>
class LFStack
{
private:
	// Internal node type
	struct Node
	{
		std::shared_ptr<T> data;
		Node* next;
		Node(T data_) : data(std::make_shared<T>(std::move(data_))) {}
	};
	// Head and thread counter
	std::atomic<Node*> head {nullptr};
	std::atomic<Node*> nodesToDelete{ nullptr };
	static std::atomic<unsigned> threadsInPop;

public:
	// push data using the CAS(compare and swap) operation
	void push(T data) {
		Node* newNode{ new Node(std::move(data)) };
		newNode->next = head.load();
		while (!head.compare_exchange_weak(newNode->next, newNode));
	}

	// try popping a stored data instance
	// return a std::shared_ptr<T> object pointing to the retrieved data
	// return an empty std::shared_ptr<T> object if the stack is empty
	std::shared_ptr<T> tryPop() {
		++threadsInPop;
		Node* popped{ head.load() };
		while (popped && !head.compare_exchange_weak(popped, popped->next));
		std::shared_ptr<T> data;
		if (popped) {
			std::swap(data, popped->data);
		}
		tryReclaim(popped);
		return data;
	}

	// try popping a stored data instance
	// return a bool variable indicating whether the retrieval was successful or not
	// retrieved data will be stored in the given reference if successful 
	bool tryPop(T& result) {
		++threadsInPop;
		Node* popped{ head.load() };
		while (popped && !head.compare_exchange_weak(popped, popped->next));
		bool valid{ false };
		if (popped) {
			result = std::move(*(popped->data));
			valid = true;
		}
		tryReclaim(popped);
		return valid;
	}

private:
	// Reclaim node memory if there is only one thread running the pop operation
	void tryReclaim(Node* node) {
		if (node) {
			chainToBeDeletedNode(node);
			if (threadsInPop == 1) {
				Node* toDelete = nodesToDelete.exchange(nullptr);
				if (--threadsInPop) {
					chainToBeDeletedNode(toDelete, findLastNode(toDelete));
				}
				else {
					deleteNodes(toDelete);
				}
			}
			else
				--threadsInPop;
		}
		else
			--threadsInPop;
	}

	// find the last node of the given node
	Node* findLastNode(Node* node) {
		Node* last = node;
		while (Node* next = last->next) {
			last = next;
		}
		return last;
	}

	// free nodes
	void deleteNodes(Node* node) {
		while (node) {
			Node* next = node->next;
			delete node;
			node = next;
		}
	}

	// chain a given node to the to be deleted node list
	void chainToBeDeletedNode(Node* node) {
		chainToBeDeletedNode(node, node);
	}

	// chain given nodes to the to be deleted node list
	void chainToBeDeletedNode(Node* head, Node* tail) {
		tail->next = nodesToDelete.load();
		while (!nodesToDelete.compare_exchange_weak(tail->next, head));
	}

};

// Init statit member
template<class T>
std::atomic<unsigned> LFStack<T>::threadsInPop {0};
