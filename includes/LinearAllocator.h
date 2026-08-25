#ifndef LINEARALLOCATOR_H
#define LINEARALLOCATOR_H

#include "Allocator.h"

class LinearAllocator : public Allocator {
protected:
    void* m_start_ptr;
    std::size_t m_offset;

public:
    explicit LinearAllocator(std::size_t totalSize) noexcept;
    virtual ~LinearAllocator() noexcept;

    virtual void* Allocate(std::size_t size, std::size_t alignment = 0);

    virtual void Free(void* ptr);

    virtual void Init();
    
    void Reset() noexcept;
};

#endif /* LINEARALLOCATOR_H */