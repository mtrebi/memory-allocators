// D-38: 本文件是 DoublyLinkedList.h 的实现片段，只供其包含，不应单独包含。
// 它没有 include guard —— 重复保护由父头文件提供。

#include "DoublyLinkedList.h"

template <class T>
DoublyLinkedList<T>::DoublyLinkedList() {

}

template <class T>
void DoublyLinkedList<T>::insert(Node* previousNode, Node* newNode) {
    // D-49: 原实现在"尾插"分支里只接了 next，漏了 newNode->previous。
    // 该字段留着调用方的垃圾值，之后 remove(newNode) 读它判断"是不是首节点"，
    // 走错分支后把 head 或别的节点的 next 改坏 —— 链表静默断裂。
    // 现在三个分支都显式写满 newNode 的 previous / next。
    if (previousNode == nullptr) {
        // Is the first node
        newNode->previous = nullptr;
        newNode->next = head;
        if (head != nullptr) {
            head->previous = newNode;
        }
        head = newNode;
    } else {
        newNode->previous = previousNode;
        newNode->next = previousNode->next;
        if (newNode->next != nullptr) {
            newNode->next->previous = newNode;
        }
        previousNode->next = newNode;
    }
}

template <class T>
void DoublyLinkedList<T>::remove(Node* deleteNode) {
    // 防御：空指针 / 已摘除节点（两个指针都为空的孤立节点）直接返回，
    // 否则下面会读到已释放内存或把 head 改坏。
    if (deleteNode == nullptr) return;

    Node* prev = deleteNode->previous;
    Node* next = deleteNode->next;

    if (prev == nullptr) {
        // 已摘除的孤立节点（prev/next 都为空）重复 remove 时，head 早已不是它。
        // 这里必须挡住，否则会把 head 置空、整条链表被清掉。
        if (head != deleteNode) return;
        head = next;
        if (next != nullptr) next->previous = nullptr;
    } else {
        prev->next = next;
        if (next != nullptr) next->previous = prev;
    }

    // 摘除后把节点自身的指针清空：这样重复 remove 同一节点会落进
    // prev == next == nullptr 的"孤立节点"情形（head 不变），而不是
    // 拿着悬空指针再改一遍链表。
    deleteNode->previous = nullptr;
    deleteNode->next = nullptr;
}