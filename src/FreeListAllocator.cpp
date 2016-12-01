#include "FreeListAllocator.h"
#include "Utils.h"		/* CalculatePaddingWithHeader */
#include <stdlib.h>     /* malloc, free */
#include <cassert> 		/* assert		*/
#include <limits>		/* limits_max */

#ifdef _DEBUG
	#include <iostream>
#endif

FreeListAllocator::FreeListAllocator(const std::size_t totalSize, PlacementPolicy pPolicy, StoragePolicy sPolicy)
	: Allocator(totalSize) {
	m_pPolicy = pPolicy;
	m_sPolicy = sPolicy;
}

void FreeListAllocator::Init() {
	//if (m_freeList != nullptr){
	//	delete m_freeList;
	//}
	if (m_start_ptr != nullptr){
		free(m_start_ptr);
		m_start_ptr = nullptr;
	}
	m_start_ptr = malloc(m_totalSize);

	this->Reset();
}

FreeListAllocator::~FreeListAllocator(){
	free(m_start_ptr);
	m_start_ptr = nullptr;
}

void* FreeListAllocator::Allocate(const std::size_t size, const std::size_t alignment){
	// Search through the free list for a free block that has enough space to allocate our data
	std::size_t padding;
	Node<FreeListAllocator::BlockHeader> * affectedNode = this->Find(size, alignment, padding);

	if (padding > 0){
		// Create a free block with the padding
		Node<FreeListAllocator::BlockHeader> * paddedBlock = (Node<FreeListAllocator::BlockHeader> *) affectedNode;
		paddedBlock->data = FreeListAllocator::BlockHeader { padding };
		m_freeList.insert(affectedNode->previous, paddedBlock);
	}

	const std::size_t rest = affectedNode->data.blockSize - padding - sizeof(FreeListAllocator::BlockHeader);

	if (rest > 0){
		// We have to split the block into the data block and a free block of size 'rest'
		Node<FreeListAllocator::BlockHeader> * newFreeNode = (Node<FreeListAllocator::BlockHeader> *)((std::size_t) affectedNode + size + 1);
		newFreeNode->data = FreeListAllocator::BlockHeader { rest };
		m_freeList.insert(affectedNode, newFreeNode);
	}
	// Setup the data block
	m_freeList.remove(affectedNode);

	const std::size_t headerAddress = (std::size_t) affectedNode + padding;
	const std::size_t dataAddress 	= headerAddress + sizeof(FreeListAllocator::BlockHeader) + 1;
	((FreeListAllocator::BlockHeader *) headerAddress)->blockSize = size + sizeof(FreeListAllocator::BlockHeader);

#ifdef _DEBUG
	std::cout << "A" << "\t@C " << (void*) affectedBlock << std::endl;
#endif

	return (void*) dataAddress;
}

Node<FreeListAllocator::BlockHeader> * FreeListAllocator::Find(const std::size_t size, const std::size_t alignment, std::size_t& padding){
	switch(m_pPolicy){
		case FIND_FIRST:
			return FindFirst(size, alignment, padding);
		case FIND_BEST:
			return FindBest(size, alignment, padding);
		default:
			return nullptr;
	}
}

Node<FreeListAllocator::BlockHeader> * FreeListAllocator::FindFirst(const std::size_t size, const std::size_t alignment, std::size_t& padding){
	//Iterate list and return the first free block with a size >= than given size
	Node<FreeListAllocator::BlockHeader> * it = m_freeList.head;
	while(it != nullptr){
		padding = Utils::CalculatePaddingWithHeader((std::size_t)it, alignment, sizeof(FreeListAllocator::BlockHeader));
		const std::size_t requiredSpace = size + padding;
		if (it->data.blockSize >= requiredSpace){
			return it;
		}
		it = it->next;
	}
	return nullptr;
}

Node<FreeListAllocator::BlockHeader> * FreeListAllocator::FindBest(const std::size_t size, const std::size_t alignment, std::size_t& padding){
	// Iterate WHOLE list keeping a pointer to the best fit
	std::size_t smallestDiff = 	std::numeric_limits<std::size_t>::max();
	Node<FreeListAllocator::BlockHeader> * bestBlock = nullptr;
	Node<FreeListAllocator::BlockHeader> * it = m_freeList.head;
	while(it != nullptr){
		padding = Utils::CalculatePaddingWithHeader((std::size_t)it, alignment, sizeof(FreeListAllocator::BlockHeader));
		const std::size_t requiredSpace = size + padding;
		if (it->data.blockSize >= requiredSpace && (it->data.blockSize - requiredSpace < smallestDiff)){
			bestBlock = it;
		}
		it = it->next;
	}
	return bestBlock;
}

void FreeListAllocator::Free(void* ptr){
	Node<FreeListAllocator::BlockHeader> * freeNode = InsertFree(ptr);
	switch(m_sPolicy){
		case SORTED:
			Coalescence(freeNode);
			break;
	}
}

Node<FreeListAllocator::BlockHeader> * FreeListAllocator::InsertFree(void * ptr){
	switch(m_sPolicy){
		case LIFO:
			return InsertFreeLIFO(ptr);
		case SORTED:
			return InsertFreeSorted(ptr);
	}
}

Node<FreeListAllocator::BlockHeader> * FreeListAllocator::InsertFreeLIFO(void * ptr){
	// Insert it in a LIFO (or stack) fashion (at the beginning)
	const std::size_t currentAddress = (std::size_t) ptr;
	const std::size_t headerAddress = currentAddress - sizeof(FreeListAllocator::BlockHeader);
	const FreeListAllocator::BlockHeader * allocationHeader { (FreeListAllocator::BlockHeader *) headerAddress};

	Node<FreeListAllocator::BlockHeader> * freeNode = (Node<FreeListAllocator::BlockHeader> *) ptr;
	freeNode->data = *allocationHeader;
	m_freeList.insert(nullptr, freeNode);

	return freeNode;
}

Node<FreeListAllocator::BlockHeader> * FreeListAllocator::InsertFreeSorted(void * ptr){
	// Insert it in a sorted position by the address number
	const std::size_t currentAddress = (std::size_t) ptr;
	const std::size_t headerAddress = currentAddress - sizeof(FreeListAllocator::BlockHeader);
	const FreeListAllocator::BlockHeader * allocationHeader { (FreeListAllocator::BlockHeader *) headerAddress};

	Node<FreeListAllocator::BlockHeader> * freeNode = (Node<FreeListAllocator::BlockHeader> *) ptr;
	freeNode->data = *allocationHeader;

	Node<FreeListAllocator::BlockHeader> * it = m_freeList.head;
	while(it != nullptr){
		if ((std::size_t) ptr < (std::size_t) it->data.blockSize){
			m_freeList.insert(it->previous, freeNode);
			break;
		}
		it = it->next;
	}
	return freeNode;

}

void FreeListAllocator::Coalescence(Node<FreeListAllocator::BlockHeader> * freeNode){
	if ((std::size_t) freeNode->previous + 
		(std::size_t) freeNode->previous->data.blockSize + 1 == (std::size_t) freeNode){
		freeNode->data.blockSize = freeNode->previous->data.blockSize + freeNode->data.blockSize;
		m_freeList.remove(freeNode->previous);
	}

	if ((std::size_t) freeNode + 
		(std::size_t) freeNode->data.blockSize + 1 == (std::size_t) freeNode->next){
		freeNode->data.blockSize = freeNode->next->data.blockSize + freeNode->data.blockSize;
		m_freeList.remove(freeNode->next);
	}
}

void FreeListAllocator::Reset() {
	Node<FreeListAllocator::BlockHeader> * firstNode = (Node<FreeListAllocator::BlockHeader> *) m_start_ptr;
	firstNode->data.blockSize = m_totalSize;
	firstNode->previous = nullptr;
	firstNode->next = nullptr;

	// Init linkedlist with only one element
	//m_freeList = DoublyLinkedList<FreeListAllocator::BlockHeader>();
	m_freeList.insert(nullptr, firstNode);
}