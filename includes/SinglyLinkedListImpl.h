#include "SinglyLinkedList.h"

template <class T>
SinglyLinkedList<T>::SinglyLinkedList() {

}

template <class T>
void SinglyLinkedList<T>::push(T element, void * ptr) {
    Node* newNode = (Node*) ptr;
    newNode->data = element;
    
    newNode->next = head;
    head = newNode;
}

template <class T>
T SinglyLinkedList<T>::pop() {
    Node * top = head;
    head = head->next;
    return top->data;
}