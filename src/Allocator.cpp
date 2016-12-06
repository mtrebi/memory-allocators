#include "Allocator.h"
#include <cassert> //assert

Allocator::Allocator(const std::size_t totalSize){
    m_totalSize = totalSize;
    m_used = 0;
}

Allocator::~Allocator(){
    m_totalSize = 0;
}