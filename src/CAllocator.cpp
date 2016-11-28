#include "CAllocator.h"
#include <stdlib.h>     /* malloc, free */

CAllocator::CAllocator()
    : Allocator(0) {

}

void CAllocator::Init() {

}

CAllocator::~CAllocator(){
    
}

void* CAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
	return malloc(size);
}

void CAllocator::Free(void* ptr) {
	free(ptr);
}


