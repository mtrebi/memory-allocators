// D-38: 本文件是 SinglyLinkedList.h 的实现片段，只供其包含，不应单独包含。
// 它没有 include guard —— 重复保护由父头文件提供。

#include "SinglyLinkedList.h"

// D-03: 原实现为空函数体，head 从未被赋值。现由类内成员初始化器
// 与 "= default" 承担，此处不再定义构造函数，避免重复定义。

template <class T>
void SinglyLinkedList<T>::insert(Node* previousNode, Node* newNode){
    if (previousNode == nullptr) {
        // Is the first node
        if (head != nullptr) {
            // The list has more elements
            newNode->next = head;           
        }else {
            newNode->next = nullptr;
        }
        head = newNode;
    } else {
        if (previousNode->next == nullptr){
            // Is the last node
            previousNode->next = newNode;
            newNode->next = nullptr;
        }else {
            // Is a middle node
            newNode->next = previousNode->next;
            previousNode->next = newNode;
        }
    }
}

template <class T>
void SinglyLinkedList<T>::remove(Node* previousNode, Node* deleteNode){
    if (previousNode == nullptr){
        // Is the first node
        if (deleteNode->next == nullptr){
            // List only has one element
            head = nullptr;            
        }else {
            // List has more elements
            head = deleteNode->next;
        }
    }else {
        previousNode->next = deleteNode->next;
    }
}