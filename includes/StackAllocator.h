#include "Allocator.h"

#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H

class StackAllocator : public Allocator {
protected:
	/* Offset from the start of the memory block */
	std::size_t m_offset;
public:
	/* Allocation of real memory */
	StackAllocator(const std::size_t totalSize);

	/* Frees all memory */
	virtual ~StackAllocator();

	/* Allocate virtual memory */
	virtual void* Allocate(const std::size_t size, const std::size_t alignment) override;

	/* Frees virtual memory */
	virtual void Free(void* ptr, const std::size_t size);

	virtual void Free(void* ptr) override;

	/* Frees all virtual memory */
	virtual void Reset() override;

private:
	StackAllocator(StackAllocator &stackAllocator);

private:
	const std::size_t CalculatePadding(const std::size_t offset, const std::size_t alignment);
};

#endif /* STACKALLOCATOR_H */