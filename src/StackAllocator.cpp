#include "StackAllocator.h"
#include <iostream>


StackAllocator::StackAllocator(const std::size_t totalSize)
	: Allocator(totalSize) {
	
	m_offset = 0;
}

StackAllocator::~StackAllocator() {
	// Do nothing
}
/*
struct Padding {
	char p;
};*/

void* StackAllocator::Allocate(const std::size_t size, const std::size_t alignment){
	std::size_t padding = 0;
	std::size_t paddedAddress = 0;
	const std::size_t currentAddress = (std::size_t)m_start_ptr + m_offset;

	if (alignment!= 0 && m_offset % alignment != 0) {
		// Alignment is required. Find the next aligned memory address and update offset
		padding = CalculatePadding(m_offset, alignment);
		m_offset += padding;
	}

	const std::size_t nextAddress = (std::size_t) m_start_ptr + m_offset;

	if (padding > 0){
		paddedAddress = (std::size_t) nextAddress  - 1;
		*(int *) paddedAddress = padding;
	}

	m_offset += size;

#ifdef 	DEBUG
	std::cout << "\t\tAddress " << (void*)nextAddress  << "\tOffset " << m_offset << "\tPadding " << padding << "\tSize " << size << "\tAlignment " << alignment << "\tDPadded " << (void*) paddedAddress<< std::endl;
#endif
	return (void*) nextAddress;
}

void StackAllocator::Free(void* ptr) {

}


void StackAllocator::Free(void* ptr, const std::size_t size) {
	// Move offset back to clear address
	m_offset -= size;
	const std::size_t currentAddress = (std::size_t) m_start_ptr + m_offset;
	const std::size_t paddedAddress = currentAddress - 1;
	const int padding = *(int *) paddedAddress;

	if (padding > 0) {
		// There was padding - Move offset back to clear padding
		m_offset -= padding;
	}

#ifdef 	DEBUG
	std::cout << "\t\tAddress " << (void*)currentAddress  << "\tOffset " << m_offset << "\tPadding " << padding << "\tDPadded " << (void*) paddedAddress<<  std::endl;
#endif

}

void StackAllocator::Reset() {
	m_offset = 0;
}

const std::size_t StackAllocator::CalculatePadding(const std::size_t offset, const std::size_t alignment) {
	const std::size_t multiplier = (offset / alignment) + 1;
	const std::size_t padding = (multiplier * alignment) - offset;
	return padding;
}
