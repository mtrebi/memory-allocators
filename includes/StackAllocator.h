#include "LinearAllocator.h"

#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H

class StackAllocator : public LinearAllocator {
public:
	/* Allocation of real memory */
	StackAllocator(const long totalSize);

	/* Frees all memory */
	virtual ~StackAllocator();

	/* Allocate virtual memory */
	virtual void* Allocate(const std::size_t size, const std::size_t alignment) override;

	/* Frees virtual memory */
	virtual void Free(void* ptr) override;

};

#endif /* STACKALLOCATOR_H */