#ifndef SINGLYLINKEDLIST_H
#define SINGLYLINKEDLIST_H

template <class T>
class SinglyLinkedList {
public:
    struct Node {
        T data;
        Node * next;
    };
    
    // D-03: 原实现中 head 未初始化，默认构造后指向不确定地址。
    // 首次 insert 时 "if (head != nullptr)" 的判断读取的是野指针。
    Node * head = nullptr;
    
public:
    // D-34: 链表持有的是外部内存（Node 存放在被管理的堆区内部），
    // 一旦按值拷贝就会出现两个链表共用同一批节点，任一方的插入/摘除
    // 都会破坏另一方。明确禁用拷贝语义。
    SinglyLinkedList() = default;
    SinglyLinkedList(const SinglyLinkedList&) = delete;
    SinglyLinkedList& operator=(const SinglyLinkedList&) = delete;

    void insert(Node * previousNode, Node * newNode);
    void remove(Node * previousNode, Node * deleteNode);
};

#include "SinglyLinkedListImpl.h"

#endif /* SINGLYLINKEDLIST_H */
