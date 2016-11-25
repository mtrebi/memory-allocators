#include "LinearAllocatorUnaligned.h"
#include <iostream>

LinearAllocatorUnaligned::LinearAllocatorUnaligned(const std::size_t totalSize) {
	m_totalSize = totalSize;
	m_start_ptr = malloc(m_totalSize);
	m_offset = 0;
}

LinearAllocatorUnaligned::~LinearAllocatorUnaligned(){
	free(m_start_ptr);
}

void* LinearAllocatorUnaligned::Allocate(const std::size_t size){
	const std::size_t currentAddress = (std::size_t)m_start_ptr + m_offset;
	m_offset += size;
	const std::size_t nextAddress = (std::size_t) m_start_ptr + m_offset;

	if (m_offset > m_totalSize){
		return nullptr;
	}

	return (void*) nextAddress;
}

void LinearAllocatorUnaligned::Reset() {
	m_offset = 0;
}
