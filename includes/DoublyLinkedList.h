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
	//void split(Node<T>* splitNode, Node<T>* splittedNode1, Node<T>* splittedNode2);
	//Node<T>* merge(Node<T>* node1, Node<T>* node2);
private:
	DoublyLinkedList(FreeListAllocator &freeListAllocator);
	DoublyLinkedList operator=(const DoublyLinkedList&);
};

#endif /* DOUBLYLINKEDLIST_H */
/*
 	Double-Linked List generic methods: 
		GENERIC DATA TYPE (TEMPLATE)
		INSERT (AFTER_PTR, PTR);
		DELETE (PTR);
		SPLIT (PTR, OUT_PTR1, OUT_PTR2);
		MERGE(PTR1, PTR2)
*/

