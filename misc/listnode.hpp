#ifndef LISTNODE_HPP
#define LISTNODE_HPP

template<typename T>

struct ListNode {
 	T value;
	ListNode *next;
	ListNode(T &val) : value(val), next(nullptr){};
	ListNode(T &val, ListNode* _next) : value(val), next(_next){};
};

#endif
