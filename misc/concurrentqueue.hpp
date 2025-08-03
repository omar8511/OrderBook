#ifndef CONCURRENTQUEUE_HPP
#define CONCURRENTQUEUE_HPP

#include "listnode.hpp"

#include <mutex>    

// size_t adapts to platform size
// Upgrade to optionals

template<typename T>

class ConcurrentQueue{

public:
	ConcurrentQueue(const T& headElement);
	ConcurrentQueue() : size(0), head(nullptr), tail(nullptr) {};
	~ConcurrentQueue();
	const T& peek() const; // Only see the first element returns an unmodifiable reference to the original object
	T pop(); // Pop off first element
	void push(const T &toPush);
	bool isEmpty() const; // Second const gives a promise that the queue wont be modified
	


private:
	size_t size;	
	mutable std::mutex mtx; // mutable allows modificaiton in const methods eg in isEmpty or peek
	ListNode* head;
	ListNode* tail;
};

#endif
