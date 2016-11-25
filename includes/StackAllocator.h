#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H

#include "LinearAllocator.h"

struct Padding {
	char padding;
};

class StackAllocator : public LinearAllocator {
public:
	/* Allocation of real memory */
	StackAllocator(const std::size_t totalSize);

	/* Frees all memory */
	virtual ~StackAllocator();

	/* Allocate virtual memory */
	virtual void* Allocate(const std::size_t size, const std::size_t alignment);

	/* Frees memory from the top of the stack */
	virtual void Free(const std::size_t size);

	/* Frees all virtual memory */
	virtual void Reset() override;

private:
	StackAllocator(StackAllocator &stackAllocator);
};

#endif /* STACKALLOCATOR_H */