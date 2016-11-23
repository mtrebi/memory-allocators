#include "Allocator.h"
#include <stdlib.h>     /* malloc, free */

Allocator::Allocator(const long totalSize){
	m_totalSize = totalSize;
	m_start_ptr = malloc(m_totalSize);
}

Allocator::~Allocator(){
	free(m_start_ptr);
}