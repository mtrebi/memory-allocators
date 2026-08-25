#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H

#include "Allocator.h"
#include <vector>

class StackAllocator : public Allocator {
protected:
    void* m_start_ptr;
    std::size_t m_offset;
    std::vector<std::size_t> m_markers;

public:
    explicit StackAllocator(std::size_t totalSize) noexcept;
    virtual ~StackAllocator() noexcept;

    virtual void* Allocate(std::size_t size, std::size_t alignment = 0);

    virtual void Free(void* ptr);

    virtual void Init();

    void Reset() noexcept;
    void Pop(std::size_t marker) noexcept;
};

#endif /* STACKALLOCATOR_H */