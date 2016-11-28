#include "PoolAllocator.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>     /* malloc, free */

#ifdef _DEBUG
  #include <iostream>
#endif

PoolAllocator::LinkedStack::LinkedStack()
{

}

PoolAllocator::LinkedStack::~LinkedStack(){
  free(this->head);
}

void PoolAllocator::LinkedStack::Push(void* freePosition){
  StackNode *_topNode = (StackNode *) freePosition;
  
  _topNode->next = this->head;
  this->head = _topNode;

#ifdef _DEBUG
  std::cout << "Pool @ " << (void*) this->head << "\tNext " << (void*) this->head->next << std::endl;
#endif

}

void *PoolAllocator::LinkedStack::Pop()
{
  assert(this->head != NULL && "The pool allocator is full");

  StackNode *_topNode = this->head;
  this->head = _topNode->next;
  return (void *) _topNode;
}

PoolAllocator::PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize) 
  : Allocator(totalSize) {
  assert(chunkSize >= 8 && "Chunk size must be greater or equal to 8");
  assert(totalSize % chunkSize == 0 && "Total Size must be a multiple of Chunk Size" );
  this->m_chunkSize = chunkSize;
}

void PoolAllocator::Init(){
  this->head = new StackNode();
  this->head->next = NULL;
  m_start_ptr = malloc(m_totalSize);

  // Create a linked-list with all free positions
  freeStack.head = NULL;
  const int nChunks = m_totalSize / m_chunkSize;
  for (int i = 0; i < nChunks; ++i){
    freeStack.Push((void *)(std::size_t)this->m_start_ptr + i * m_chunkSize);
  }
}

PoolAllocator::~PoolAllocator(){
  free(m_start_ptr);
}


void *PoolAllocator::Allocate(const std::size_t allocationSize, const std::size_t alignment)
{
  assert(allocationSize == this->m_chunkSize && "Allocation size must be equal to chunk size");

  void * freePosition = this->freeStack.Pop();

  assert(freePosition != NULL && "The pool allocator is full");

  this->m_used += m_chunkSize;

#ifdef _DEBUG
  std::cout << "A" << "\t@S " << m_start_ptr << "\t@R " << freePosition << "\tM " << m_used << std::endl;
#endif

  return freePosition;
}

void PoolAllocator::Free(void * ptr)
{
  this->m_used -= m_chunkSize;

  this->freeStack.Push(ptr);

#ifdef _DEBUG
  std::cout << "F" << "\t@S " << m_start_ptr << "\t@F " << ptr << "\tM " << m_used << std::endl;
#endif
}

void PoolAllocator::Reset()
{
  freeStack.head = NULL;
  for (std::size_t chunkptr = 0;
       (std::size_t)this->m_start_ptr + chunkptr < (std::size_t)this->m_totalSize;
       chunkptr += this->m_chunkSize)
  {
    freeStack.Push((void *)((std::size_t)this->m_start_ptr + chunkptr));
  }
}