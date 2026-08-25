#ifndef SINGLYLINKEDLIST_H
#define SINGLYLINKEDLIST_H

#include <cstddef>

template <class T>
class SinglyLinkedList {
public:
    struct Node {
        T data;
        Node* next;
    };

    Node* head;

    SinglyLinkedList() : head(NULL) {}

    SinglyLinkedList(const SinglyLinkedList&) = delete;
    SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

    void insert(Node* previousNode, Node* newNode) noexcept;
    void remove(Node* previousNode, Node* deleteNode) noexcept;
};

#include "SinglyLinkedListImpl.h"

#endif /* SINGLYLINKEDLIST_H */