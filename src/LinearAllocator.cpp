#include "LinearAllocator.h"


LinearAllocator::LinearAllocator(const long totalSize)
	: Allocator(totalSize) {

	m_offset = 0;
}


LinearAllocator::~LinearAllocator(){
	// Do nothing - Parent frees memory for us
}

void* LinearAllocator::Allocate(const std::size_t size){
	const long currentAddress = (long)m_start_ptr + m_offset;
	const std::size_t alignment = this->calculateAlignment(currentAddress, size);
	if (m_offset % alignment != 0) {
		// Alignment is required. Find the next aligned memory address and update offset
		const uint32_t multiplier = (m_offset / alignment) + 1;
		m_offset += multiplier * alignment;
	}
	// Offset is pointing to an aligned memory address
	const long nextAddress = (long) m_start_ptr + m_offset;
	m_offset += size;
	return (void*) nextAddress;
}

void LinearAllocator::Free(void* ptr) {
	// Do nothing - A linear allocator does not frees individual memory blocks
}

void LinearAllocator::Reset() {
	m_offset = 0;
}

const uint32_t LinearAllocator::calculateAlignment(const long address, const std::size_t size) const {
	long mask = size - 1;
	long t = address & mask;
	return address + (size - t);
}
