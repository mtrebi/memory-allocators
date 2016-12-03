#include "SinglyLinkedList.h"

template <class T>
SinglyLinkedList<T>::SinglyLinkedList() {

}

template <class T>
void SinglyLinkedList<T>::push(Node * newNode) {
    newNode->next = head;
    head = newNode;
}

template <class T>
typename SinglyLinkedList<T>::Node* SinglyLinkedList<T>::pop() {
    Node * top = head;
    head = head->next;
    return top;
}