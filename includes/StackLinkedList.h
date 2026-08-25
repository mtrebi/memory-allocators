#ifndef STACKLINKEDLIST_H
#define STACKLINKEDLIST_H

#include <cassert>

template <class T>
class StackLinkedList {
public:
    struct Node {
        T data;
        Node* next;
    };

    Node* head;

    StackLinkedList() : head(NULL) {}

    StackLinkedList(const StackLinkedList&) = delete;
    StackLinkedList& operator=(const StackLinkedList&) = delete;

    void push(Node* newNode) noexcept;
    Node* pop() noexcept;

    bool Empty() const noexcept {
        return head == NULL;
    }
};

#include "StackLinkedListImpl.h"

#endif /* STACKLINKEDLIST_H */