#ifndef CALLOCATOR_H
#define CALLOCATOR_H

#include "Allocator.h"

class CAllocator : public Allocator {
private:
    bool m_lastWasAligned;

public:
    CAllocator() noexcept;

    virtual ~CAllocator() noexcept;

    virtual void* Allocate(std::size_t size, std::size_t alignment = 0);

    virtual void Free(void* ptr);

    virtual void Init();
};

#endif /* CALLOCATOR_H */