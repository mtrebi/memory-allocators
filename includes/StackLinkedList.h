#ifndef STACKLINKEDLIST_H
#define STACKLINKEDLIST_H

template <class T>
class StackLinkedList {
public:
    struct Node {
        T data;
        Node* next;
    };
    
    Node* head;
public:
    StackLinkedList();

    void push(Node * newNode);
    Node* pop();
private:
    StackLinkedList(StackLinkedList &stackLinkedList);
};

#include "StackLinkedListImpl.h"

#endif /* STACKLINKEDLIST_H */

