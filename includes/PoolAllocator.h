#ifndef POOLALLOCATOR_H
#define POOLALLOCATOR_H

#include "Allocator.h"
#include "StackLinkedList.h"

class PoolAllocator : public Allocator {
private:
    struct FreeHeader {};
    typedef StackLinkedList<FreeHeader>::Node Node;

    StackLinkedList<FreeHeader> m_freeList;
    void* m_start_ptr;
    std::size_t m_chunkSize;

public:
    PoolAllocator(std::size_t totalSize, std::size_t chunkSize) noexcept;
    virtual ~PoolAllocator() noexcept;

    virtual void* Allocate(std::size_t size, std::size_t alignment = 0);

    virtual void Free(void* ptr);

    virtual void Init();

    void Reset() noexcept;
};

#endif