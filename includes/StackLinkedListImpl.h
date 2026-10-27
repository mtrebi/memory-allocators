// D-38: 本文件是 StackLinkedList.h 的实现片段，只供其包含，不应单独包含。
// 它没有 include guard —— 重复保护由父头文件提供。

#include "StackLinkedList.h"

template <class T>
void StackLinkedList<T>::push(Node * newNode) {
    newNode->next = head;
    head = newNode;
}

template <class T>
typename StackLinkedList<T>::Node* StackLinkedList<T>::pop() {
    if (head == nullptr) {
        return nullptr;
    }

    Node * top = head;
    head = head->next;
    return top;
}
