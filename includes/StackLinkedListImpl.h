#include "StackLinkedList.h"

template <class T>
void StackLinkedList<T>::push(Node* newNode) noexcept {
    newNode->next = head;
    head = newNode;
}

template <class T>
typename StackLinkedList<T>::Node* StackLinkedList<T>::pop() noexcept {
    assert(head != NULL && "Pop on empty 'StackLinkedList'.");

    Node* top = head;
    head = head->next;

    return top;
}