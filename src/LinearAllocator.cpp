#include "LinearAllocator.h"
#include "Utils.h"		/* CalculatePadding */
#include <stdlib.h>     /* malloc, free */
#include <cassert> 		/*assert		*/


LinearAllocator::LinearAllocator(const std::size_t totalSize)
	: Allocator(totalSize) {
}

void LinearAllocator::Init() {
	m_start_ptr = malloc(m_totalSize);
	m_offset = 0;
}


LinearAllocator::~LinearAllocator(){
	free(m_start_ptr);
	m_start_ptr = nullptr;
}

void* LinearAllocator::Allocate(const std::size_t size, const short alignment){
	int padding = 0;
	std::size_t paddedAddress = 0;
	const std::size_t currentAddress = (std::size_t)m_start_ptr + m_offset;

	if (alignment!= 0 && m_offset % alignment != 0) {
		// Alignment is required. Find the next aligned memory address and update offset
		padding = Utils::CalculatePadding((std::size_t) m_start_ptr + m_offset, alignment);
		m_offset += padding;
	}

	const std::size_t nextAddress = (std::size_t) m_start_ptr + m_offset;

	m_offset += size;
	m_used = m_offset;
	if (m_offset > m_totalSize){
		return nullptr;
	}

	return (void*) nextAddress;
}

void LinearAllocator::Free(void* ptr){
	assert( false && "Use Reset() method" );
}

void LinearAllocator::Reset() {
	m_offset = 0;
	m_used = 0;
}
