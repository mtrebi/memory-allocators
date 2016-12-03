#ifndef SIMPLYLINKEDLIST_H
#define SIMPLYLINKEDLIST_H

template <class T>
class SinglyLinkedList {
public:
    struct Node {
        T data;
        Node* next;
    };
    
    Node* head;
public:
    SinglyLinkedList();

    void push(Node * newNode);
    Node* pop();
private:
    SinglyLinkedList(SinglyLinkedList &singlyLinkedList);
};

#include "SinglyLinkedListImpl.h"

#endif /* SIMPLYLINKEDLIST_H */

