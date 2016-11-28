#include "PoolAllocator.h"
#include <assert.h>
#include <stdint.h>

PoolAllocator::LinkedStack::LinkedStack()
{
  this->topNode->next = NULL;
}

PoolAllocator::PoolAllocator::~LinkedStack(){

}

void PoolAllocator::LinkedStack::Push(void* freePosition){
  StackNode *_topNode = (StackNode *) freePosition;
  
  _topNode->next = this->topNode;
  this->topNode = _topNode;
}

void *PoolAllocator::LinkedStack::Pop()
{
  assert(this->topNode != NULL);

  StackNode *_topNode = this->topNode;
  this->topNode = this->topNode->next;
  
  return (void *) _topNode;
}

PoolAllocator::PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize) 
  : Allocator(totalSize) {
  assert(totalSize % chunkSize == 0 && "Total Size must be a multiple of Chunk Size" );
  this->m_chunkSize = chunkSize;
  this->Reset();
}

void PoolAllocator::Init(){
  //m_start_ptr = malloc(m_totalSize);
}

PoolAllocator::~PoolAllocator(){
  //Free everything -> Reset?
}


void *PoolAllocator::Allocate(const std::size_t allocationSize, const std::size_t alignment)
{
  assert(allocationSize < this->m_chunkSize);
  assert(alignment >= 0);
  void * freePosition = this->freeStack.Pop();
  assert(freePosition != NULL);

  return freePosition;
}

void PoolAllocator::Free(void * ptr)
{
  this->freeStack.Push(ptr);
}

void PoolAllocator::Reset()
{
  freeStack.topNode = NULL;
  for (std::size_t chunkptr = 0;
       (std::size_t)this->m_start_ptr + chunkptr < (std::size_t)this->m_totalSize;
       chunkptr += this->m_chunkSize)
  {
    freeStack.Push((void *)((std::size_t)this->m_start_ptr + chunkptr));
  }
}