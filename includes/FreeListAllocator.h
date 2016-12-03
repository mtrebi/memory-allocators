#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

#include "Allocator.h"
#include "SinglyLinkedList.h"

class FreeListAllocator : public Allocator {
public:
    enum PlacementPolicy {
        FIND_FIRST,
        FIND_BEST
    };

    enum StoragePolicy {
        LIFO,
        SORTED
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
    StoragePolicy m_sPolicy;
    SinglyLinkedList<FreeHeader> m_freeList;

public:
    FreeListAllocator(const std::size_t totalSize, PlacementPolicy pPolicy, StoragePolicy sPolicy);

    virtual ~FreeListAllocator();

    virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;

    virtual void Free(void* ptr) override;

    virtual void Init() override;

    virtual void Reset();
private:
    FreeListAllocator(FreeListAllocator &freeListAllocator);

    void Coalescence(Node* prevBlock, Node * freeBlock);

    Node * InsertFree(void * ptr);
    Node * InsertFreeLIFO(void * ptr);
    Node * InsertFreeSorted(void * ptr);

    void Find(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode);
    void FindBest(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode);
    void FindFirst(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode);
};

#endif /* FREELISTALLOCATOR_H */

