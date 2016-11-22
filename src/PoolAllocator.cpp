#include "Allocator.h"
#include <assert.h>
#include <stdint.h>

class PoolAllocator : public Allocator {
private:
  class LinkedStack {
  private:
    struct StackNode;
 
    struct StackNode {
      StackNode * next;
    } topNode;

  public:
    LinkedStack();
    void Push(void* freePosition);
    void * Pop();
    ~LinkedStack();

  } freeStack;

  uint32_t m_chunkSize;

public:
  /* Allocation of real memory */
  PoolAllocator(const uint32_t totalSize, const uint32_t chunkSize);

  /* Allocate virtual memory */
  void *Allocate(const std::size_t allocationSize);
  
  /* Frees a chunk of virtual memory */
  void Free(void* ptr);

  /* Frees all virtual memory */
  void Reset();

  /* Free all memory */
  ~PoolAllocator();
};

PoolAllocator::LinkedStack::LinkedStack()
{
  this->topNode.next = NULL;
}

void PoolAllocator::LinkedStack::Push(void* freePosition){
  StackNode *topNode = (StackNode *) freePosition;
  
  TopNode->next = this.topNode;
}

void PoolAllocator::LinkedStack::Pop()
{
  assert(this.topNode != NULL);

  StackNode topNode = this->topNode;
  this->topNode = this->topNode->next;
  
  return (void *) topNode;
}

PoolAllocator::PoolAllocator(const uint32_t totalSize, const uint32_t chunkSize) : Allocator(totalSize)
{
  assert(totalSize % chunkSize = 0);
  this->m_chunkSize = chunkSize;
  this->Init();
}

void * PoolAllocator::Allocate(const std::size_t allocationSize)
{
  assert(allocationSize < this->m_chunkSize);
  void * freePosition = freeStack.Pop();
  assert(freePosition != NULL);

  return freePosition;
}

void PoolAllocator::Free(void * ptr)s
{
  this->freeList.Push(ptr);
}

void PoolAllocator::Reset()
{
  freeStack.topNode = NULL;
  for (void* chunkptr = 0;
       this->m_startptr + chunkptr < this.m_totalSize;
       chunkptr += this->m_chunkSize)
  {
    freeStack.Push(this->m_startptr + chunkptr);
  }
}