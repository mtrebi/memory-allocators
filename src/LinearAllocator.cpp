#include "LinearAllocator.h"
#include <iostream>
#include <assert.h>

LinearAllocator::LinearAllocator(const long totalSize)
	: Allocator(totalSize) {
	
	m_offset = 0;
}

LinearAllocator::~LinearAllocator(){
	// Do nothing - Parent frees memory for us
}

void* LinearAllocator::Allocate(const std::size_t size, const std::size_t alignment){
	const long currentAddress = (long)m_start_ptr + m_offset;
	uint32_t displacement = 0;
	if (m_offset % alignment != 0) {
		// Alignment is required. Find the next aligned memory address and update offset
		const long multiplier = (m_offset / alignment) + 1;
		displacement = (multiplier * alignment) - m_offset;
		m_offset += (multiplier * alignment) - m_offset;
	}
	// Offset is pointing to an aligned memory address
	const long nextAddress = (long) m_start_ptr + m_offset;
	m_offset += size;
#ifdef 	DEBUG
	std::cout << "\tAdress\t\t" << (void*)nextAddress << "\tSize\t" << size << "\tOffset\t" << m_offset << "\tDisplacement\t" << displacement << "\tAlignment\t" << alignment<< std::endl;
#endif

	return (void*) nextAddress;
}

void LinearAllocator::Free(void* ptr) {
	assert(ptr==ptr);// Do nothing - A linear allocator does not frees individual memory blocks
}

void LinearAllocator::Reset() {
	m_offset = 0;
}