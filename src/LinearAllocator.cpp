#include "LinearAllocator.h"


LinearAllocator::LinearAllocator(const uint32_t totalSize)
	: Allocator(totalSize) {

	m_offset = 0;
}


LinearAllocator::~LinearAllocator(){
	// Do nothing - Parent frees memory for us
}

void* LinearAllocator::Allocate(const std::size_t allocationSize){
	// Calculate alignment
	// Update offset to match alignment
	// Update offset with the alocationSize
	// Return a pointer to the offset with alignment
}

void LinearAllocator::Free(void* ptr) {
	// Do nothing - A linear allocator does not frees individual memory blocks
}

void LinearAllocator::Reset() {
	m_offset = 0;
}