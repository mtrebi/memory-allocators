#ifndef SIMPLYLINKEDLIST_H
#define SIMPLYLINKEDLIST_H

template <class T>
class SinglyLinkedList {
private:
    struct Node {
        T data;
        Node* next;
    };
public:

    
    Node* head;
public:
    SinglyLinkedList();

    void push(T element, void *ptr);
    T pop();
private:
    SinglyLinkedList(SinglyLinkedList &singlyLinkedList);
};

#include "SinglyLinkedListImpl.h"

#endif /* SIMPLYLINKEDLIST_H */

