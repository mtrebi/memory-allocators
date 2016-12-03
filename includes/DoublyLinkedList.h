#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

template <class T>
class DoublyLinkedList {
    friend class FreeListAllocator;
private:
    struct Node {
        T data;
        Node* previous;
        Node* next;
    };
public:
    Node* head;
public:
    DoublyLinkedList();

    void insert(Node* previousNode, Node* newNode);
    void remove(Node* deleteNode);
private:
    DoublyLinkedList(DoublyLinkedList &doublyLinkedList);
};

#include "DoublyLinkedListImpl.h"

#endif /* DOUBLYLINKEDLIST_H */

