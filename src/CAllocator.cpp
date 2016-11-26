#include "CAllocator.h"
#include <stdlib.h>     /* malloc, free */

CAllocator::CAllocator(const std::size_t totalSize)
    : Allocator(totalSize) {

}

CAllocator::~CAllocator(){
    
}

void* CAllocator::Allocate(const std::size_t size, const short alignment) {
	return malloc(size);
}

void CAllocator::Free(void* ptr) {
	free(ptr);
}


