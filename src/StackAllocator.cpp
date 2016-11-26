#include "StackAllocator.h"
#include "Utils.h"		/* CalculatePadding */
#include <stdlib.h>     /* malloc, free */

StackAllocator::StackAllocator(const std::size_t totalSize)
	: Allocator(totalSize) {

}

void LinearAllocator::Init() {
	m_start_ptr = malloc(m_totalSize);
	m_offset = 0;
}

StackAllocator::~StackAllocator() {
	free(m_start_ptr);
	m_start_ptr = nullptr;
}

void* StackAllocator::Allocate(const std::size_t size, const short alignment){
	int padding = 0;
	std::size_t paddedAddress = 0;
	const std::size_t currentAddress = (std::size_t)m_start_ptr + m_offset;

	if (alignment!= 0 && m_soffset % alignment != 0) {
		// Alignment is required. Find the next aligned memory address and update offset
		padding = Utils::CalculatePadding(m_offset, alignment);
		m_offset += padding;
	}

	const std::size_t nextAddress = (std::size_t) m_start_ptr + m_offset;

	if (padding > 0){
		// Store padding size in the padding itself
		paddedAddress = (std::size_t) nextAddress  - 1;
		AllocationHeader allocationHeader {padding};
		*(int *) paddedAddress = allocationHeader.padding;
	}

	m_offset += size;

	if (m_offset > m_totalSize){
		return nullptr;
	}

	return (void*) nextAddress;
}

void StackAllocator::Free(const std::size_t size) {
	// Move offset back to clear address
	m_offset -= size;
	const std::size_t currentAddress = (std::size_t) m_start_ptr + m_offset;
	const std::size_t paddedAddress = currentAddress - 1;
	const AllocationHeader allocationHeader { *(int *) paddedAddress};

	if (allocationHeader.padding > 0) {
		// There was padding - Move offset back to clear padding
		m_offset -= allocationHeader.padding;
	}
}