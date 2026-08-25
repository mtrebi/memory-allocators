#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

#include "Allocator.h"
#include "SinglyLinkedList.h"

class FreeListAllocator : public Allocator {
public:
    enum class PlacementPolicy {
        FIND_FIRST,
        FIND_BEST
    };

private:
    struct FreeHeader {
        std::size_t blockSize;
    };
    struct AllocationHeader {
        std::size_t blockSize;
        std::size_t padding;
    };

    typedef SinglyLinkedList<FreeHeader>::Node Node;

    void* m_start_ptr;
    PlacementPolicy m_pPolicy;
    SinglyLinkedList<FreeHeader> m_freeList;

public:
    FreeListAllocator(std::size_t totalSize, PlacementPolicy pPolicy) noexcept;
    virtual ~FreeListAllocator() noexcept;

    virtual void* Allocate(std::size_t size, std::size_t alignment = 0);

    virtual void Free(void* ptr);

    virtual void Init();

    void Reset() noexcept;

private:
    void Coalescence(Node* prevBlock, Node* freeBlock) noexcept;

    void Find(std::size_t size, std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode) noexcept;
    void FindFirst(std::size_t size, std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode) noexcept;
    void FindBest(std::size_t size, std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode) noexcept;
};

#endif /* FREELISTALLOCATOR_H */