#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

template<typename N>
struct Node
{
	N data;
	Node* previous;
	Node* next;
};

template <class T>
class DoublyLinkedList {
public:
	Node<T>* head;
public:
	DoublyLinkedList();

	void insert(Node<T>* previousNode, Node<T>* newNode);
	void remove(Node<T>* deleteNode);
private:
	DoublyLinkedList(DoublyLinkedList &doublyLinkedList);
};

#include "DoublyLinkedListImpl.h"

#endif /* DOUBLYLINKEDLIST_H */

