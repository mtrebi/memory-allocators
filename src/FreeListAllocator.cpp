#include "FreeListAllocator.h"
#include "Utils.h"		/* CalculatePaddingWithHeader */
#include <stdlib.h>     /* malloc, free */
#include <cassert> 		/* assert		*/
#include <limits>		/* limits_max */
#ifdef _DEBUG
	#include <iostream>
#endif

FreeListAllocator::FreeListAllocator(const std::size_t totalSize)
	: Allocator(totalSize) {
}

void FreeListAllocator::Init() {
	if (m_freeList != nullptr){
		delete m_freeList;
	}
	if (m_start_ptr != nullptr){
		free(m_start_ptr);
		m_start_ptr = nullptr;
	}
	// Init linkedlist with only one element
	m_start_ptr = malloc(m_totalSize);

	m_freeList = (FreeBlock *) m_start_ptr;
	m_freeList = new FreeList();
	m_freeList.m_head->size = m_totalSize;
	m_freeList.m_head->previous = NULL;
	m_freeList.m_head->next = NULL;
}

FreeListAllocator::~FreeListAllocator(){
	free(m_start_ptr);
	m_start_ptr = nullptr;
}

void* FreeListAllocator::Allocate(const std::size_t size, const std::size_t alignment){
	// Search through the free list for a free block that has a size equals or bigger than the required one
	// When found, if the size is bigger, split the block into two and return one of them

	FreeBlock * affectedBlock = this->FindFirst(size);



#ifdef _DEBUGz
	std::cout << "A" << "\t@C " << (void*) currentAddress << "\t@R " << (void*) nextAddress << "\tO " << m_offset << "\tP " << padding << std::endl;
#endif
	
	return (void*) nextAddress;
}

FreeBlock * FreeListAllocator::FindFirst(const std::size_t size){
	//Iterate list and return the first free block with a size >= than given size
	FreeBlock * it = m_freeList;
	while(it != nullptr){
		if (it->size >= size){
			return it;
		}
		it = it->next;
	}
	return nullptr;
}

FreeBlock * FreeListAllocator::FindBest(const std::size_t size){
	// Iterate WHOLE list keeping a pointer to the best fit
	std::size_t smallestDiff = 	std::numeric_limits<T>::max();
	FreeBlock * bestBlock = nullptr;
	FreeBlock * it = m_freeList;
	while(it != nullptr){
		if (it->size >= size && (it->size - size < smallestDiff)){
			bestBlock = it;
		}
		it = it->next;
	}
	return bestBlock;
}
void FreeListAllocator::Free(void* ptr){
	//TODO
}

void FreeListAllocator::Reset() {
	// TODO:Set all adresses on the linked list as free
}
