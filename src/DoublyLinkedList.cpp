#include "DoublyLinkedList.h"

template <class T>
DoublyLinkedList<T>::DoublyLinkedList(){
	
}
template <class T>
void DoublyLinkedList<T>::insert(Node<T>* previousNode, Node<T>* newNode){
	if (previousNode == nullptr){
		// Should be inserted at the beginning
		if (head != nullptr){
			// There are more nodes
			newNode->next = head->next;
			newNode->next->previous = newNode;
		}
		head = newNode;
		newNode->previous = nullptr;
	}
	else if (previousNode->next == nullptr) {
		// Should be inserted at the end
		previousNode->next = newNode;
		newNode->previous = previousNode;
		newNode->next = nullptr;
	}
}
template <class T>
void DoublyLinkedList<T>::remove(Node<T>* deleteNode){
	if (deleteNode->previousNode != nullptr){
		// Is not the first node
		deleteNode->previousNode->next = deleteNode->next;
	}
	if (deleteNode->next != nullptr){
		// Is not the last node
		deleteNode->next->previous = deleteNode->previousNode;
	}
}