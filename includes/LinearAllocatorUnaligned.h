#ifndef LINEARALLOCATORUNALIGNED_H
#define LINEARALLOCATORUNALIGNED_H

#include <cstddef> // std::size_t

class LinearAllocatorUnaligned {
protected:
	/* Memory size */
	std::size_t m_totalSize;
	/* Pointer to the start of the memory block. Once set, never changes */
	void* m_start_ptr;
	/* Offset from the start of the memory block */
	std::size_t m_offset;
public:
	/* Allocation of real memory */
	LinearAllocatorUnaligned(const std::size_t totalSize);

	/* Frees all memory */
	virtual ~LinearAllocatorUnaligned();

	/* Allocate virtual memory */
	virtual void* Allocate(const std::size_t size);

	/* Frees all virtual memory */
	virtual void Reset();
private:
	LinearAllocatorUnaligned(LinearAllocatorUnaligned &linearAllocatorUnaligned);
};

#endif /* LINEARALLOCATORUNALIGNED_H */
