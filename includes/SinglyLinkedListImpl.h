#include "SinglyLinkedList.h"

template <class T>
void SinglyLinkedList<T>::insert(Node* previousNode, Node* newNode) noexcept {
    if (previousNode == NULL) {
        newNode->next = head;
        head = newNode;
    }
    else if (previousNode->next == NULL) {
        previousNode->next = newNode;
        newNode->next = NULL;
    }
    else {
        newNode->next = previousNode->next;
        previousNode->next = newNode;
    }
}

template <class T>
void SinglyLinkedList<T>::remove(Node* previousNode, Node* deleteNode) noexcept {
    if (previousNode == NULL)
        head = deleteNode->next;
    else
        previousNode->next = deleteNode->next;
}