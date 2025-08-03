#include "concurrentqueue.hpp"
#include "listnode.hpp"

#include <mutex>
#include <stdexcept>


ConcurrentQueue::ConcurrentQueue(const T& headElement){
	head = new ListNode(headElement);
	size = 1;
	tail = this->head;
}

const T& ConcurrenQueue::peek(){
	std::lock_guard<std::mutex> lock(mtx); // Auto releases	
	if(isEmpty()) throw std::runtime_error("Tried to Peek an Empty Stack");
	return &head->value;
}

T ConcurrentQueue::pop(){
	std::lock_guard<std::mutex> lock(mtx); 
	if(isEmpty()) throw std::runtime_error("Tried to pop an empty queue");
	if(head == tail) {
		T res = head->value;
		delete head;
		head = nullptr;
		tail = nullptr;
		return res;
	} else {
		T res = head->value;
		ListNode<T>* temp = head;
		head = head->next;		
		delete temp;
		return res;
	}
	size--;
}

void ConcurrentQueue::push(const T& toPush){
	std::lock_guard<std::mutex> lock(mtx);
	if(isEmpty()) {
		head = new ListNode<T>(toPush);
		tail = head;
	} else {
		tail->next = new ListNode<T>(toPush);
		tail = tail->next;
	}
	size++;
}


bool ConcurrentQueue::isEmpty(){
	std::lock_guard<std::mutex> lock(mtx); 
	return size == 0;
}

