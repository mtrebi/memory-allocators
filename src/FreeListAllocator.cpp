#include "FreeListAllocator.h"
#include "Utils.h"  /* CalculatePaddingWithHeader */
#include <cstdlib>
#include <cassert>   /* assert		*/
#include <limits>  /* limits_max */
#include <algorithm>    // std::max

#ifdef _DEBUG
#include <iostream>
#endif

FreeListAllocator::FreeListAllocator(const std::size_t totalSize, const PlacementPolicy pPolicy) noexcept : Allocator(totalSize), m_start_ptr(NULL), m_pPolicy(pPolicy) {}

void FreeListAllocator::Init() {
    if (m_start_ptr != NULL) {
        std::free(m_start_ptr);
        m_start_ptr = NULL;
    }
    m_start_ptr = std::malloc(m_totalSize);

    assert(m_start_ptr != NULL && "FreeListAllocator: malloc failed.");

    Reset();
}

FreeListAllocator::~FreeListAllocator() noexcept {
    std::free(m_start_ptr);
    m_start_ptr = NULL;
}

void* FreeListAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
    assert(m_start_ptr != NULL && "FreeListAllocator::Allocate called before 'Init()'.");
    assert(size >= sizeof(Node) && "Allocation size must be '>= sizeof(Node)'.");
    assert(alignment >= 8 && "Alignment must be at least 8.");
    assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of 2.");

    const std::size_t allocationHeaderSize = sizeof(AllocationHeader);

    std::size_t padding = 0;
    Node* affectedNode = NULL;
    Node* previousNode = NULL;
    Find(size, alignment, padding, previousNode, affectedNode);

    assert(affectedNode != NULL && "FreeListAllocator: not enough memory.");

    const std::size_t alignmentPadding = padding - allocationHeaderSize;
    const std::size_t requiredSize = size + padding;
    const std::size_t rest = affectedNode->data.blockSize - requiredSize;

    if (rest >= sizeof(Node)) {
        Node* newFreeNode = reinterpret_cast<Node*>(reinterpret_cast<std::size_t>(affectedNode) + requiredSize);

        newFreeNode->data.blockSize = rest;
        m_freeList.insert(affectedNode, newFreeNode);
    }
    m_freeList.remove(previousNode, affectedNode);

    const std::size_t headerAddress = reinterpret_cast<std::size_t>(affectedNode) + alignmentPadding;
    const std::size_t dataAddress = headerAddress + allocationHeaderSize;

    AllocationHeader* header = reinterpret_cast<AllocationHeader*>(headerAddress);

    header->blockSize = requiredSize + (rest < sizeof(Node) ? rest : 0);
    header->padding = alignmentPadding;

    m_used += header->blockSize;
    m_peak = std::max(m_peak, m_used);

#ifdef _DEBUG
    std::cout << "A\t@H " << reinterpret_cast<void*>(headerAddress) << "\tD@ " << reinterpret_cast<void*>(dataAddress) << "\tS " << header->blockSize << "\tAP " << alignmentPadding << "\tP " << padding << "\tM " << m_used << "\tR " << rest << '\n';
#endif

    return reinterpret_cast<void*>(dataAddress);
}

void FreeListAllocator::Find(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode) noexcept {
    switch (m_pPolicy) {
        case PlacementPolicy::FIND_FIRST:
            FindFirst(size, alignment, padding, previousNode, foundNode);
            break;
        case PlacementPolicy::FIND_BEST:
            FindBest(size, alignment, padding, previousNode, foundNode);
            break;
    }
}

void FreeListAllocator::FindFirst(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode) noexcept {
    Node* it = m_freeList.head;
    Node* itPrev = NULL;

    while (it != NULL) {
        padding = Utils::CalculatePaddingWithHeader(reinterpret_cast<std::size_t>(it), alignment, sizeof(AllocationHeader));

        if (it->data.blockSize >= size + padding)
            break;

        itPrev = it;
        it = it->next;
    }

    previousNode = itPrev;
    foundNode = it;
}

void FreeListAllocator::FindBest(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode) noexcept {
    std::size_t smallestDiff = std::numeric_limits<std::size_t>::max();
    Node* bestBlock = NULL;
    Node* bestPrev = NULL;
    std::size_t bestPadding = 0;

    Node* it = m_freeList.head;
    Node* itPrev = NULL;

    while (it != NULL) {
        const std::size_t currentPadding = Utils::CalculatePaddingWithHeader(reinterpret_cast<std::size_t>(it), alignment, sizeof(AllocationHeader));
        const std::size_t requiredSpace = size + currentPadding;

        if (it->data.blockSize >= requiredSpace) {
            const std::size_t diff = it->data.blockSize - requiredSpace;

            if (diff < smallestDiff) {
                smallestDiff = diff;
                bestBlock = it;
                bestPrev = itPrev;
                bestPadding = currentPadding;
            }
        }

        itPrev = it;
        it = it->next;
    }

    padding = bestPadding;
    previousNode = bestPrev;
    foundNode = bestBlock;
}

void FreeListAllocator::Free(void* ptr) {
    assert(ptr != NULL && "FreeListAllocator::Free: null pointer.");

    const std::size_t currentAddress = reinterpret_cast<std::size_t>(ptr);
    const std::size_t headerAddress = currentAddress - sizeof(AllocationHeader);
    const AllocationHeader* allocationHeader = reinterpret_cast<const AllocationHeader*>(headerAddress);

    Node* freeNode = reinterpret_cast<Node*>(headerAddress - allocationHeader->padding);
    freeNode->data.blockSize = allocationHeader->blockSize + allocationHeader->padding;
    freeNode->next = NULL;

    Node* it = m_freeList.head;
    Node* itPrev = NULL;

    while (it != NULL && reinterpret_cast<std::size_t>(it) < reinterpret_cast<std::size_t>(freeNode)) {
        itPrev = it;
        it = it->next;
    }

    m_freeList.insert(itPrev, freeNode);

    m_used -= freeNode->data.blockSize;

    // Merge contiguous nodes
    Coalescence(itPrev, freeNode);

#ifdef _DEBUG
    std::cout << "F\t@ptr " << ptr << "\tH@ " << static_cast<void*>(freeNode) << "\tS " << freeNode->data.blockSize << "\tM " << m_used << '\n';
#endif
}

void FreeListAllocator::Coalescence(Node* previousNode, Node* freeNode) noexcept {
    if (freeNode->next != NULL && reinterpret_cast<std::size_t>(freeNode) + freeNode->data.blockSize == reinterpret_cast<std::size_t>(freeNode->next)) {
        freeNode->data.blockSize += freeNode->next->data.blockSize;
        m_freeList.remove(freeNode, freeNode->next);
#ifdef _DEBUG
        std::cout << "\tMerging(n) " << static_cast<void*>(freeNode) << " S " << freeNode->data.blockSize << '\n';
#endif
    }

    if (previousNode != NULL && reinterpret_cast<std::size_t>(previousNode) + previousNode->data.blockSize == reinterpret_cast<std::size_t>(freeNode)) {
        previousNode->data.blockSize += freeNode->data.blockSize;
        m_freeList.remove(previousNode, freeNode);
#ifdef _DEBUG
        std::cout << "\tMerging(p) " << static_cast<void*>(previousNode) << " S " << previousNode->data.blockSize << '\n';
#endif
    }
}

void FreeListAllocator::Reset() noexcept {
    m_used = 0;
    m_peak = 0;
    m_freeList.head = NULL;
    Node* firstNode = reinterpret_cast<Node*>(m_start_ptr);
    firstNode->data.blockSize = m_totalSize;
    firstNode->next = NULL;
    m_freeList.insert(NULL, firstNode);
}