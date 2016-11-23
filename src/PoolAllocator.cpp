#include "Allocator.h"
#include <assert.h>
#include <stdint.h>

class PoolAllocator : public Allocator {
private:
  class LinkedStack {
  public:
    struct StackNode;
 
    struct StackNode {
      StackNode * next;
    } * topNode;

    LinkedStack();
    void Push(void* freePosition);
    void * Pop();
    ~LinkedStack();

  } freeStack;

  std::size_t m_chunkSize;

public:
  /* Allocation of real memory */
  PoolAllocator(const uint32_t totalSize, const uint32_t chunkSize);

  /* Allocate virtual memory */
  void *Allocate(const std::size_t allocationSize, const std::size_t alignment);
  
  /* Frees a chunk of virtual memory */
  void Free(void* ptr);

  /* Frees all virtual memory */
  void Reset();

  /* Free all memory */
  ~PoolAllocator();
};

PoolAllocator::LinkedStack::LinkedStack()
{
  this->topNode->next = NULL;
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

PoolAllocator::PoolAllocator(const uint32_t totalSize, const uint32_t chunkSize) : Allocator(totalSize)
{
  assert(totalSize % chunkSize == 0);
  this->m_chunkSize = chunkSize;
  this->Reset();
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