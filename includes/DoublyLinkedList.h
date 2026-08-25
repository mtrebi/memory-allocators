#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include <cstddef>

template <class T>
class DoublyLinkedList {
public:
    struct Node {
        T data;
        Node* previous;
        Node* next;
    };
    Node* head;

    DoublyLinkedList() : head(NULL) {}

    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    void insert(Node* previousNode, Node* newNode) noexcept;
    void remove(Node* deleteNode) noexcept;
};

#include "DoublyLinkedListImpl.h"

#endif /* DOUBLYLINKEDLIST_H */