#include "LinearAllocator.h"
#include "Utils.h"  /* CalculatePadding */
#include <cstdlib>
#include <cassert>   /*assert		*/
#include <algorithm>    // max
#ifdef _DEBUG
#include <iostream>
#endif

LinearAllocator::LinearAllocator(const std::size_t totalSize) noexcept : Allocator(totalSize), m_start_ptr(NULL), m_offset(0) {}

void LinearAllocator::Init() {
    if (m_start_ptr != NULL)
        std::free(m_start_ptr);

    m_start_ptr = std::malloc(m_totalSize);

    assert(m_start_ptr != NULL && "LinearAllocator: malloc failed.");

    m_offset = 0;
}

LinearAllocator::~LinearAllocator() noexcept {
    std::free(m_start_ptr);
    m_start_ptr = NULL;
}

void* LinearAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
    assert(m_start_ptr != NULL && "LinearAllocator::Allocate called before 'Init()'.");
    assert(size > 0 && "Allocation size must be greater than 0.");

    const std::size_t currentAddress = reinterpret_cast<std::size_t>(m_start_ptr) + m_offset;
    std::size_t padding = 0;

    if (alignment > 1 && (currentAddress & (alignment - 1)) != 0)
        padding = Utils::CalculatePadding(currentAddress, alignment);

    if (m_offset + padding + size > m_totalSize)
        return NULL;

    m_offset += padding + size;
    m_used = m_offset;
    m_peak = std::max(m_peak, m_used);

    const std::size_t nextAddress = currentAddress + padding;

#ifdef _DEBUG
    std::cout << "A\t@C " << reinterpret_cast<void*>(currentAddress) << "\t@R " << reinterpret_cast<void*>(nextAddress) << "\tO " << m_offset << "\tP " << padding << '\n';
#endif

    return reinterpret_cast<void*>(nextAddress);
}

void LinearAllocator::Free(void* ptr) {
    (void)ptr;

    assert(false && "LinearAllocator does not support individual frees - use 'Reset()'.");
}

void LinearAllocator::Reset() noexcept {
    m_offset = 0;
    m_used = 0;
}