#include "FreeListAllocator.h"
#include "Utils.h"		/* CalculatePaddingWithHeader */
#include <stdlib.h>     /* malloc, free */
#include <cassert> 		/*assert		*/

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
	FreeBlock * affectedBlock = this->FindFirst(size);



#ifdef _DEBUGz
	std::cout << "A" << "\t@C " << (void*) currentAddress << "\t@R " << (void*) nextAddress << "\tO " << m_offset << "\tP " << padding << std::endl;
#endif
	
	return (void*) nextAddress;
}

FreeBlock * FreeListAllocator::FindFirst(const std::size_t){
	//TODO: Iterate list and return the first free block with a size >= than given size
}

FreeBlock * FreeListAllocator::FindBest(const std::size_t){
	//TODO: Iterate WHOLE list keeping a pointer to the best fit
	return nullptr;
}
void FreeListAllocator::Free(void* ptr){
	//TODO
}

void FreeListAllocator::Reset() {
	// TODO:Set all adresses on the linked list as free
}
