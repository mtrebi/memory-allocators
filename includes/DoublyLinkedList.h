#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

// D-22: 死代码。本仓库没有任何翻译单元包含它。
//
// 它的来历：README 的 "Red black tree data structure" 一节描述的方案需要
// "一条额外有序双向链表，用于在 O(1) 内完成合并"。这个类是为那个尚未实现
// 的方案预留的。红黑树 free list 至今仍在 Future work 里，所以它从未被接入。
//
// 保留而不删除的理由：它是上述设计的直接产物，删掉会让 README 的那段描述
// 失去对应物。若要清理，可连同 README 该节一起删除。
//
// D-48 / D-49：本文件此前从未被任何用例编译过，因此两个缺陷一直没暴露。
// 现在由 tests/probe_round6.cpp 的 doubly 用例覆盖（insert 头/中/尾、remove
// 头/中/尾、单元素、正反向指针一致性）。

template <class T>
class DoublyLinkedList {
public:
    struct Node {
        T data;
        Node* previous;
        Node* next;
    };
    // D-48: 原实现只写 "Node* head;" 且构造函数体为空 —— head 是未定义值。
    // 与 SinglyLinkedList 的 D-03 完全同类，只是这份代码没人用所以没暴露。
    Node* head = nullptr;
public:
    DoublyLinkedList();

    void insert(Node* previousNode, Node* newNode);
    void remove(Node* deleteNode);
private:
    DoublyLinkedList(DoublyLinkedList &doublyLinkedList);
};

#include "DoublyLinkedListImpl.h"

#endif /* DOUBLYLINKEDLIST_H */

