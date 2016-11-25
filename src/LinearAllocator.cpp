#include "LinearAllocator.h"
#include <iostream>

LinearAllocator::LinearAllocator(const std::size_t totalSize) {
	m_totalSize = totalSize;
	m_start_ptr = malloc(m_totalSize);
	m_offset = 0;
}

LinearAllocator::~LinearAllocator(){
	free(m_start_ptr);
}

void* LinearAllocator::Allocate(const std::size_t size, const std::size_t alignment){
int padding = 0;
	std::size_t paddedAddress = 0;
	const std::size_t currentAddress = (std::size_t)m_start_ptr + m_offset;

	if (alignment!= 0 && m_offset % alignment != 0) {
		// Alignment is required. Find the next aligned memory address and update offset
		padding = CalculatePadding(m_offset, alignment);
		m_offset += padding;
	}

	const std::size_t nextAddress = (std::size_t) m_start_ptr + m_offset;

	m_offset += size;

	if (m_offset > m_totalSize){
		return nullptr;
	}

	return (void*) nextAddress;
}

void LinearAllocator::Reset() {
	m_offset = 0;
}

const std::size_t LinearAllocator::CalculatePadding(const std::size_t offset, const std::size_t alignment) {
	const std::size_t multiplier = (offset / alignment) + 1;
	const std::size_t padding = (multiplier * alignment) - offset;
	return padding;
}
