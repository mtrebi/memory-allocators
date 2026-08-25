#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstddef>

class Allocator {
protected:
    std::size_t m_totalSize;
    std::size_t m_used;
    std::size_t m_peak;

public:
    explicit Allocator(const std::size_t totalSize) noexcept : m_totalSize(totalSize), m_used(0), m_peak(0) {}

    virtual ~Allocator() noexcept {
        m_totalSize = 0;
    }

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;

    virtual void* Allocate(std::size_t size, std::size_t alignment = 0) = 0;

    virtual void Free(void* ptr) = 0;

    virtual void Init() = 0;

    friend class Benchmark;
};

#endif /* ALLOCATOR_H */