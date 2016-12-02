#include "DoublyLinkedList.h"

template <class T>
DoublyLinkedList<T>::DoublyLinkedList() {

}

template <class T>
void DoublyLinkedList<T>::insert(Node<T>* previousNode, Node<T>* newNode) {
    if (previousNode == nullptr) {
        // Is the first node
        if (head != nullptr) {
            // The list has more elements
            newNode->next = head;           
            newNode->next->previous = newNode;
        }else {
            newNode->next = nullptr;
        }
        head = newNode;
        head->previous = nullptr;
    } else {
        if (previousNode->next == nullptr){
            // Is the last node
            previousNode->next = newNode;
            newNode->next = nullptr;
        }else {
            // Is a middle node
            newNode->next = previousNode->next;
            if (newNode->next != nullptr){
                newNode->next->previous = newNode;
            }
            previousNode->next = newNode;
            newNode->previous = previousNode;
        }
    }
}

template <class T>
void DoublyLinkedList<T>::remove(Node<T>* deleteNode) {
    if (deleteNode->previous == nullptr){
        // Is the first node
        if (deleteNode->next == nullptr){
            // List only has one element
            head = nullptr;            
        }else {
            // List has more elements
            head = deleteNode->next;
            head->previous = nullptr;
        }
    }else {
        if (deleteNode->next == nullptr){
            // Is the last node
            deleteNode->previous->next = nullptr;
        }else {
            // Middle node
            deleteNode->previous->next = deleteNode->next;
            deleteNode->next->previous = deleteNode->previous;
        }
    }
    
    
    if (deleteNode->previous != nullptr) {
        // Is not the first node
        deleteNode->previous->next = deleteNode->next;
    } else {
        // Is the first node
        head = deleteNode->next;
        if (deleteNode->next != nullptr) {
            deleteNode->next->previous = nullptr;
        }
    }
    if (deleteNode->next != nullptr) {
        // Is not the last node
        deleteNode->next->previous = deleteNode->previous;
    }else {
        // Is the last node
        if (deleteNode->previous != nullptr) {
            deleteNode->previous->next = nullptr;
        }
    }
}