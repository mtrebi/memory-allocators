#ifndef SIMPLYLINKEDLIST_H
#define SIMPLYLINKEDLIST_H

template <class T>
class SinglyLinkedList {
private:
    struct sNode {
        T data;
        sNode* next;
    };
public:

    
    sNode* head;
public:
    SinglyLinkedList();

    void push(T element, void *ptr);
    T pop();
private:
    SinglyLinkedList(SinglyLinkedList &singlyLinkedList);
};

#include "SinglyLinkedListImpl.h"

#endif /* SIMPLYLINKEDLIST_H */

