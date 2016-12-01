#include "FreeListAllocator.h"
#include "Utils.h"		/* CalculatePaddingWithHeader */
#include <stdlib.h>     /* malloc, free */
#include <cassert> 		/* assert		*/
#include <limits>		/* limits_max */

#ifdef _DEBUG
	#include <iostream>
#endif

FreeListAllocator::FreeListAllocator(const std::size_t totalSize, enum PlacementPolicy pPolicy, enum sPolicy);
	: Allocator(totalSize) {
	m_pPolicy = pPolicy;
	m_sPolicy = sPolicy;
}

void FreeListAllocator::Init() {
	if (m_freeList != nullptr){
		delete m_freeList;
	}
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
	const std::size_t requiredSpace = size + sizeof(BlockHeader); //TODO ALIGNMENT!!!!
	Node<BlockHeader> * affectedNode = this->Find(requiredSpace);
	const int rest = affectedNode->size - requiredSpace;
	if (rest > 0){
		// We have to split the block into the data block and a free block of size 'rest'
		Node<BlockHeader> * newFreeNode = (Node<BlockHeader> *)((std::size_t) affectedNode + size + 1);
		newFreeNode->data = BlockHeader { rest };
		m_freeList->insert(affectedNode, newFreeNode);
	}
	m_freeList->delete(affectedNode)

	const std::size_t headerAddress = (std::size_t) affectedNode;
	const std::size_t dataAddress 	= headerAddress + (std::size_t) + sizeof(BlockHeader) + 1;
	(*(BlockHeader *) headerAddress).blockSize = BlockHeader { requiredSpace };	//TODO ALIGNMENT!!!!


#ifdef _DEBUG
	std::cout << "A" << "\t@C " << (void*) affectedBlock << std::endl;
#endif

	return (void*) dataAddress;
}

Node<BlockHeader> * FreeListAllocator::Find(const std::size_t size){
	switch(m_pPolicy){
		case FIND_FIRST:
			return FindFirst(size);
		case FIND_BEST:
			return FindBest(size);
		default:
			return nullptr;
	}
}

Node<BlockHeader> * FreeListAllocator::FindFirst(const std::size_t size){
	//Iterate list and return the first free block with a size >= than given size
	Node<BlockHeader> * it = m_freeList->head;
	while(it != nullptr){
		if (it->size >= size){
			return it;
		}
		it = it->next;
	}
	return nullptr;
}

Node<BlockHeader> * FreeListAllocator::FindBest(const std::size_t size){
	// Iterate WHOLE list keeping a pointer to the best fit
	std::size_t smallestDiff = 	std::numeric_limits<std::size_t>::max();
	Node<BlockHeader> * bestBlock = nullptr;
	Node<BlockHeader> * it = m_freeList->head;
	while(it != nullptr){
		if (it->size >= size && (it->size - size < smallestDiff)){
			bestBlock = it;
		}
		it = it->next;
	}
	return bestBlock;
}

void FreeListAllocator::Free(void* ptr){
	Node<BlockHeader> * freeNode = InsertFree(ptr);
	Coalescence(freeNode);
}

Node<BlockHeader> * FreeListAllocator::InsertFree(void * ptr){
	switch(m_sPolicy){
		case LIFO:
			return InsertFreeLIFO(ptr);
		case SORTED:
			return InsertFreeSorted(ptr);
	}
}

Node<BlockHeader> * FreeListAllocator::InsertFreeLIFO(void * ptr){
	// Insert it in a LIFO (or stack) fashion (at the beginning)
	const std::size_t currentAddress = (std::size_t) ptr;
	const std::size_t headerAddress = currentAddress - sizeof(BlockHeader);
	const BlockHeader * allocationHeader { (BlockHeader *) headerAddress};

	Node<BlockHeader> * freeNode = (Node<BlockHeader> *) ptr;
	freeNode->data = BlockHeader { allocationHeader->blockSize };
	m_freeList.insert(nullptr, freeNode);

	return freeNode;
}

Node<BlockHeader> * FreeListAllocator::InsertFreeSorted(void * ptr){
	// Insert it in a sorted position by the address number
	const std::size_t currentAddress = (std::size_t) ptr;
	const std::size_t headerAddress = currentAddress - sizeof(BlockHeader);
	const BlockHeader * allocationHeader { (BlockHeader *) headerAddress};

	Node<BlockHeader> * freeNode = (Node<BlockHeader> *) ptr;
	freeNode->data = BlockHeader { allocationHeader->blockSize };

	Node<BlockHeader> * it = m_freeList.head;
	while(it != nullptr){
		if ((std::size_t) ptr < (std::size_t) it->data.size){
			m_freeList->insert(it->previous, freeNode);
			break;
		}
		it = it->next;
	}
	return freeNode;

}

void FreeListAllocator::Coalescence(Node<BlockHeader> * freeNode){
	if ((std::size_t) freeNode->previous + 
		(std::size_t) freeNode->previous->size + 1 == (std::size_t) freeNode){
		freeNode->data.size = BlockHeader { freeNode->previous->data.size + freeNode->data.size };
		m_freeList->delete(freeNode->previous);
	}

	if ((std::size_t) freeNode + 
		(std::size_t) freeNode->size + 1 == (std::size_t) freeNode->next){
		freeNode->data.size = BlockHeader { freeNode->next->data.size + freeNode->data.size };
		m_freeList->delete(freeNode->next);
	}
}

void FreeListAllocator::Reset() {
	Node<BlockHeader> * firstNode = (Node<BlockHeader> *) m_start_ptr;
	firstNode->data = BlockHeader { m_totalSize };
	firstNode->previous = nullptr;
	firstNode->next = nullptr;

	// Init linkedlist with only one element
	m_freeList = DoublyLinkedList<BlockHeader>(m_start_ptr);
	m_freeList->insert(nullptr, firstNode);
}

// TODO and ALIGNMENT and 