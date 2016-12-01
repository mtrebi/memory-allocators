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
	void delete(Node<T>* deleteNode);
private:
	DoublyLinkedList(FreeListAllocator &freeListAllocator);
	DoublyLinkedList operator=(const DoublyLinkedList&);
};

#endif /* DOUBLYLINKEDLIST_H */

