#include "StackAllocator.h"
#include <iostream>


StackAllocator::StackAllocator(const std::size_t totalSize)
	: LinearAllocator(totalSize) {
}

StackAllocator::~StackAllocator() {
	// Do nothing - Parent already frees memory
}

void* StackAllocator::Allocate(const std::size_t size, const std::size_t alignment){
	int padding = 0;
	std::size_t paddedAddress = 0;
	const std::size_t currentAddress = (std::size_t)m_start_ptr + m_offset;

	if (alignment!= 0 && m_offset % alignment != 0) {
		// Alignment is required. Find the next aligned memory address and update offset
		padding = CalculatePadding(m_offset, alignment);
		m_offset += padding;
	}

	const std::size_t nextAddress = (std::size_t) m_start_ptr + m_offset;

	if (padding > 0){
		// Store padding size in the padding itself
		paddedAddress = (std::size_t) nextAddress  - 1;
		Padding pad_struct {padding};
		*(int *) paddedAddress = pad_struct.padding;
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
	const Padding pad_struct { *(int *) paddedAddress};

	if (pad_struct.padding > 0) {
		// There was padding - Move offset back to clear padding
		m_offset -= pad_struct.padding;
	}
}

void StackAllocator::Reset() {
	LinearAllocator::Reset();
}