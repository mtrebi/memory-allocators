#include "Allocator.h"

class PoolAllocator : public Allocator {
private:
    void * m_start_ptr;
    std::size_t m_chunkSize;

    class LinkedStack {
    public:
        struct StackNode {
            StackNode * next;
        } * head;

        LinkedStack();
        void Push(void* freePosition);
        void * Pop();
        ~LinkedStack();
    };

    LinkedStack freeStack;

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