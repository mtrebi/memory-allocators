#include "LinearAllocator.h"
#include <iostream>

LinearAllocator::LinearAllocator(const long totalSize)
	: Allocator(totalSize) {
	
	m_offset = 0;
}


LinearAllocator::~LinearAllocator(){
	// Do nothing - Parent frees memory for us
}

void* LinearAllocator::Allocate(const std::size_t size){
	const long currentAddress = (long)m_start_ptr + m_offset;
	//const std::size_t alignment = this->calculateAlignment(currentAddress, size);
	const std::size_t alignment = size;
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
	std::cout << "Adress\t\t" << (void*)nextAddress << "\tSize\t" << size << "\tOffset\t" << m_offset << "\tDisplacement\t" << displacement << std::endl;

	return (void*) nextAddress;
}

void LinearAllocator::Free(void* ptr) {
	// Do nothing - A linear allocator does not frees individual memory blocks
}

void LinearAllocator::Reset() {
	m_offset = 0;
}

const uint32_t LinearAllocator::calculateAlignment(const long address, const std::size_t size) const {
	long mask = size*4 - 1; // 4 -> 1 
	long t = address & mask;
	uint32_t alignment = address + (size - t);
	std::cout << "address = " << address << std::endl;

	std::cout << "t = " << t << std::endl;
	std::cout << "mask = " << mask<< std::endl;
	std::cout << "alignment = " << alignment << std::endl;
	return alignment;
}
