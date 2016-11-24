#include <typeinfo>
#include <stdint.h>

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

class Allocator {
protected:
	/* Memory size */
	std::size_t m_totalSize;
	
	/* Pointer to the start of the memory block. Once set, never changes */
	void* m_start_ptr;
public:
	/* Allocation of memory */
	Allocator(const std::size_t totalSize);

	/* Frees all memory */
	virtual ~Allocator();

	/* Allocate virtual memory */
	virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) = 0;

	/* Frees virtual memory */
	virtual void Free(void* ptr) = 0;

	/* Frees all virtual memory */
	virtual void Reset() = 0;
};

#endif /* ALLOCATOR_H */
