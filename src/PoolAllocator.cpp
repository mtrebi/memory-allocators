#include "PoolAllocator.h"
#include <cstdlib>
#include <cassert>
#include <algorithm>    //max
#ifdef _DEBUG
#include <iostream>
#endif

PoolAllocator::PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize) noexcept : Allocator(totalSize), m_start_ptr(NULL), m_chunkSize(chunkSize) {
    assert(chunkSize >= sizeof(void*) && "Chunk size must be '>= sizeof(void*)'.");
    assert(totalSize % chunkSize == 0 && "Total size must be a multiple of chunk size.");
}

void PoolAllocator::Init() {
    if (m_start_ptr != NULL) {
        std::free(m_start_ptr);
        m_start_ptr = NULL;
    }

    m_start_ptr = std::malloc(m_totalSize);

    assert(m_start_ptr != NULL && "PoolAllocator: malloc failed.");

    Reset();
}

PoolAllocator::~PoolAllocator() noexcept {
    std::free(m_start_ptr);
    m_start_ptr = NULL;
}

void* PoolAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
    (void)size;
    (void)alignment;

    assert(m_start_ptr != NULL && "PoolAllocator::Allocate called before 'Init()'.");
    assert(size == m_chunkSize && "PoolAllocator: allocation size must equal chunk size.");
    assert(!m_freeList.Empty() && "PoolAllocator is full.");

    Node* freeNode = m_freeList.pop();
    m_used += m_chunkSize;
    m_peak = std::max(m_peak, m_used);

#ifdef _DEBUG
    std::cout << "A\t@S " << m_start_ptr << "\t@R " << static_cast<void*>(freeNode) << "\tM " << m_used << '\n';
#endif

    return static_cast<void*>(freeNode);
}

void PoolAllocator::Free(void* ptr) {
    assert(ptr != NULL && "PoolAllocator::Free: null pointer.");
    assert(m_used >= m_chunkSize && "PoolAllocator::Free: underflow.");

    m_used -= m_chunkSize;
    m_freeList.push(static_cast<Node*>(ptr));

#ifdef _DEBUG
    std::cout << "F\t@S " << m_start_ptr << "\t@F " << ptr << "\tM " << m_used << '\n';
#endif
}

void PoolAllocator::Reset() noexcept {
    m_used = 0;
    m_peak = 0;
    m_freeList.head = NULL;

    const std::size_t nChunks = m_totalSize / m_chunkSize;

    for (std::size_t i = 0; i < nChunks; ++i) {
        Node* node = reinterpret_cast<Node*>(reinterpret_cast<std::size_t>(m_start_ptr) + i * m_chunkSize);

        m_freeList.push(node);
    }
}