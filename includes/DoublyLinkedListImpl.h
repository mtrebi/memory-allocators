#include "DoublyLinkedList.h"

template <class T>
void DoublyLinkedList<T>::insert(Node* previousNode, Node* newNode) noexcept {
    if (previousNode == NULL) {
        newNode->previous = NULL;
        newNode->next = head;

        if (head != NULL)
            head->previous = newNode;

        head = newNode;
    }
    else {
        newNode->previous = previousNode;
        newNode->next = previousNode->next;

        if (previousNode->next != NULL)
            previousNode->next->previous = newNode;
        
        previousNode->next = newNode;
    }
}

template <class T>
void DoublyLinkedList<T>::remove(Node* deleteNode) noexcept {
    if (deleteNode->previous != NULL)
        deleteNode->previous->next = deleteNode->next;
    else
        head = deleteNode->next;

    if (deleteNode->next != NULL)
        deleteNode->next->previous = deleteNode->previous;
        
    deleteNode->previous = NULL;
    deleteNode->next = NULL;
}