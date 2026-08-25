#include "StackAllocator.h"
#include "Utils.h"
#include <cstdlib>
#include <algorithm>
#include <cassert>
#ifdef _DEBUG
#include <iostream>
#endif

StackAllocator::StackAllocator(const std::size_t totalSize) noexcept : Allocator(totalSize), m_start_ptr(NULL), m_offset(0) {}

void StackAllocator::Init() {
    if (m_start_ptr != NULL) {
        std::free(m_start_ptr);
        m_start_ptr = NULL;
    }
    m_start_ptr = std::malloc(m_totalSize);

    assert(m_start_ptr != NULL && "StackAllocator: malloc failed.");

    Reset();

    m_markers.reserve(64);
}

StackAllocator::~StackAllocator() noexcept {
    std::free(m_start_ptr);
    m_start_ptr = NULL;
}

void* StackAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
    assert(m_start_ptr != NULL && "StackAllocator::Allocate called before 'Init()'.");
    assert(size > 0 && "Allocation size must be greater than 0.");

    const std::size_t currentAddress = reinterpret_cast<std::size_t>(m_start_ptr) + m_offset;
    std::size_t padding = 0;

    if (alignment > 1)
        padding = Utils::CalculatePadding(currentAddress, alignment);
    
    if (m_offset + padding + size > m_totalSize)
        return NULL;

    m_markers.push_back(m_offset);
    const std::size_t nextAddress = currentAddress + padding;
    m_offset += padding + size;
    m_used = m_offset;
    m_peak = std::max(m_peak, m_used);

#ifdef _DEBUG
    std::cout << "A\t@C " << reinterpret_cast<void*>(currentAddress) << "\t@R " << reinterpret_cast<void*>(nextAddress) << "\tO " << m_offset << "\tP " << padding << '\n';
#endif

    return reinterpret_cast<void*>(nextAddress);
}

void StackAllocator::Free(void* ptr) {
    (void)ptr;

    assert(!m_markers.empty() && "StackAllocator::Free called with no active allocations (LIFO violation).");

    m_offset = m_markers.back();
    m_markers.pop_back();
    m_used = m_offset;

#ifdef _DEBUG
    std::cout << "F\t@F " << ptr << "\tO " << m_offset << '\n';
#endif
}

void StackAllocator::Reset() noexcept {
    m_offset = 0;
    m_used = 0;
    m_peak = 0;
    m_markers.clear();
}

void StackAllocator::Pop(const std::size_t marker) noexcept {
    assert(marker <= m_offset && "StackAllocator::Pop: marker exceeds current offset.");

    while (!m_markers.empty() && m_markers.back() > marker)
        m_markers.pop_back();

    m_offset = marker;
    m_used = m_offset;
}