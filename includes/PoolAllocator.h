#include "Allocator.h"
#include "StackLinkedList.h"

class PoolAllocator : public Allocator {
private:
    struct  FreeHeader{
    };
    typedef StackLinkedList<FreeHeader>::Node Node;
    StackLinkedList<FreeHeader> m_freeList;

    void * m_start_ptr;
    std::size_t m_chunkSize;
public:
    PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize);

    virtual ~PoolAllocator();

    virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;

    virtual void Free(void* ptr) override;

    virtual void Init() override;

    virtual void Reset();
private:
    PoolAllocator(PoolAllocator &poolAllocator);

};