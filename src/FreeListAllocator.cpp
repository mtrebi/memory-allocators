#include "FreeListAllocator.h"
#include "Utils.h"  /* CalculatePaddingWithHeader */
#include <stdlib.h>     /* malloc, free */
#include <cassert>   /* assert		*/
#include <limits>  /* limits_max */

#ifdef _DEBUG
#include <iostream>
#endif

FreeListAllocator::FreeListAllocator(const std::size_t totalSize, PlacementPolicy pPolicy, StoragePolicy sPolicy)
: Allocator(totalSize) {
    m_pPolicy = pPolicy;
    m_sPolicy = sPolicy;
}

void FreeListAllocator::Init() {
    if (m_start_ptr != nullptr) {
        free(m_start_ptr);
        m_start_ptr = nullptr;
    }
    m_start_ptr = malloc(m_totalSize);

    this->Reset();
}

FreeListAllocator::~FreeListAllocator() {
    free(m_start_ptr);
    m_start_ptr = nullptr;
}

void* FreeListAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
    const std::size_t allocationHeaderSize = sizeof(FreeListAllocator::AllocationHeader);
    const std::size_t freeHeaderSize = sizeof(FreeListAllocator::FreeHeader);
    assert("Allocation size must be bigger" && size >= sizeof(Node<FreeListAllocator::FreeHeader>));
    // Search through the free list for a free block that has enough space to allocate our data
    std::size_t padding;
    Node<FreeListAllocator::FreeHeader> * affectedNode = this->Find(size, alignment, padding);
    assert (affectedNode != nullptr && "Not enough memory");


    const std::size_t alignmentPadding =  padding - allocationHeaderSize;
    const std::size_t requiredSize = size + padding;    

    const std::size_t previousAffected = (std::size_t) affectedNode->previous;
    const std::size_t rest = affectedNode->data.blockSize - requiredSize;
    m_freeList.remove(affectedNode);

    if (rest > 0) {
        // We have to split the block into the data block and a free block of size 'rest'
        Node<FreeListAllocator::FreeHeader> * newFreeNode = (Node<FreeListAllocator::FreeHeader> *)((std::size_t) affectedNode + requiredSize);
        newFreeNode->data.blockSize = rest;
        m_freeList.insert((Node<FreeListAllocator::FreeHeader> *)previousAffected, newFreeNode);
    }
    
    // Setup data block
    const std::size_t headerAddress = (std::size_t) affectedNode + alignmentPadding;
    const std::size_t dataAddress = headerAddress + allocationHeaderSize;
    ((FreeListAllocator::AllocationHeader *) headerAddress)->blockSize = requiredSize;
    ((FreeListAllocator::AllocationHeader *) headerAddress)->padding = alignmentPadding;

    m_used += requiredSize;

#ifdef _DEBUG
    std::cout << "A" << "\t@H " << (void*) headerAddress << "\tD@ " <<(void*) dataAddress << "\tS " << ((FreeListAllocator::AllocationHeader *) headerAddress)->blockSize <<  "\tAP " << alignmentPadding << "\tP " << padding << "\tM " << m_used << "\tR " << rest << std::endl;
#endif

    return (void*) dataAddress;
}

Node<FreeListAllocator::FreeHeader> * FreeListAllocator::Find(const std::size_t size, const std::size_t alignment, std::size_t& padding) {
    switch (m_pPolicy) {
        case FIND_FIRST:
            return FindFirst(size, alignment, padding);
        case FIND_BEST:
            return FindBest(size, alignment, padding);
        default:
            return nullptr;
    }
}

Node<FreeListAllocator::FreeHeader> * FreeListAllocator::FindFirst(const std::size_t size, const std::size_t alignment, std::size_t& padding) {
    //Iterate list and return the first free block with a size >= than given size
    Node<FreeListAllocator::FreeHeader> * it = m_freeList.head;
    while (it != nullptr) {
        padding = Utils::CalculatePaddingWithHeader((std::size_t)it, alignment, sizeof (FreeListAllocator::AllocationHeader));
        const std::size_t requiredSpace = size + padding;
        if (it->data.blockSize >= requiredSpace) {
            return it;
        }
        it = it->next;
    }
    return nullptr;
}

Node<FreeListAllocator::FreeHeader> * FreeListAllocator::FindBest(const std::size_t size, const std::size_t alignment, std::size_t& padding) {
    // Iterate WHOLE list keeping a pointer to the best fit
    std::size_t smallestDiff = std::numeric_limits<std::size_t>::max();
    Node<FreeListAllocator::FreeHeader> * bestBlock = nullptr;
    Node<FreeListAllocator::FreeHeader> * it = m_freeList.head;
    while (it != nullptr) {
        padding = Utils::CalculatePaddingWithHeader((std::size_t)it, alignment, sizeof (FreeListAllocator::AllocationHeader));
        const std::size_t requiredSpace = size + padding;
        if (it->data.blockSize >= requiredSpace && (it->data.blockSize - requiredSpace < smallestDiff)) {
            bestBlock = it;
        }
        it = it->next;
    }
    return bestBlock;
}

void FreeListAllocator::Free(void* ptr) {
    Node<FreeListAllocator::FreeHeader> * freeNode = InsertFree(ptr);
    m_used -= freeNode->data.blockSize;
    
#ifdef _DEBUG
    std::cout << "F" << "\t@ptr " <<  ptr <<"\tN@ " << (void*) freeNode << "\tS " << freeNode->data.blockSize << "\tM " << m_used << std::endl;
#endif
    switch (m_sPolicy) {
        case SORTED:
            Coalescence(freeNode);
            break;
    }
}

Node<FreeListAllocator::FreeHeader> * FreeListAllocator::InsertFree(void * ptr) {
    switch (m_sPolicy) {
        case LIFO:
            return InsertFreeLIFO(ptr);
        case SORTED:
            return InsertFreeSorted(ptr);
    }
}

Node<FreeListAllocator::FreeHeader> * FreeListAllocator::InsertFreeLIFO(void * ptr) {
    // Insert it in a LIFO (or stack) fashion (at the beginning)
    const std::size_t currentAddress = (std::size_t) ptr;
    const std::size_t headerAddress = currentAddress - sizeof (FreeListAllocator::FreeHeader);
    const FreeListAllocator::FreeHeader * allocationHeader{ (FreeListAllocator::FreeHeader *) headerAddress};

    Node<FreeListAllocator::FreeHeader> * freeNode = (Node<FreeListAllocator::FreeHeader> *) ptr;
    freeNode->data = *allocationHeader;
    m_freeList.insert(nullptr, freeNode);

    return freeNode;
}

Node<FreeListAllocator::FreeHeader> * FreeListAllocator::InsertFreeSorted(void * ptr) {
    // Insert it in a sorted position by the address number
    const std::size_t currentAddress = (std::size_t) ptr;
    const std::size_t headerAddress = currentAddress - sizeof (FreeListAllocator::AllocationHeader);
    const FreeListAllocator::AllocationHeader * allocationHeader{ (FreeListAllocator::AllocationHeader *) headerAddress};

    Node<FreeListAllocator::FreeHeader> * freeNode = (Node<FreeListAllocator::FreeHeader> *) (headerAddress);
    freeNode->data.blockSize = allocationHeader->blockSize + allocationHeader->padding;
    freeNode->next = nullptr;
    freeNode->previous = nullptr;

    Node<FreeListAllocator::FreeHeader> * it = m_freeList.head;
    while (it != nullptr) {
        if (ptr < it) {
            m_freeList.insert(it->previous, freeNode);
            break;
        }
        it = it->next;
    }
    return freeNode;
}

void FreeListAllocator::Coalescence(Node<FreeListAllocator::FreeHeader> * freeNode) {   
    if (freeNode->next != nullptr && 
            (std::size_t) freeNode + freeNode->data.blockSize == (std::size_t) freeNode->next) {
        freeNode->data.blockSize += freeNode->next->data.blockSize;
        m_freeList.remove(freeNode->next);
#ifdef _DEBUG
    std::cout << "\tMerging(n) " << (void*) freeNode << " & " << (void*) freeNode->next << "\tS " << freeNode->data.blockSize << std::endl;
#endif
    
    if (freeNode->previous != nullptr &&
            (std::size_t) freeNode->previous + freeNode->previous->data.blockSize == (std::size_t) freeNode) {
        freeNode->previous->data.blockSize += freeNode->data.blockSize;
        m_freeList.remove(freeNode);
#ifdef _DEBUG
    std::cout << "\tMerging(p) " << (void*) freeNode->previous << " & " << (void*) freeNode << "\tS " << freeNode->previous->data.blockSize << std::endl;
#endif
    }


    }
}

void FreeListAllocator::Reset() {
    Node<FreeListAllocator::FreeHeader> * firstNode = (Node<FreeListAllocator::FreeHeader> *) m_start_ptr;
    firstNode->data.blockSize = m_totalSize;
    firstNode->previous = nullptr;
    firstNode->next = nullptr;

    m_freeList.insert(nullptr, firstNode);
}