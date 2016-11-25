#ifndef LINEARALLOCATOR_H
#define LINEARALLOCATOR_H

#include <cstddef> // std::size_t

class LinearAllocator {
protected:
	/* Memory size */
	std::size_t m_totalSize;
	/* Pointer to the start of the memory block. Once set, never changes */
	void* m_start_ptr;
	/* Offset from the start of the memory block */
	std::size_t m_offset;
public:
	/* Allocation of real memory */
	LinearAllocator(const std::size_t totalSize);

	/* Frees all memory */
	virtual ~LinearAllocator();

	/* Allocate virtual memory */
	virtual void* Allocate(const std::size_t size, const std::size_t alignment);

	/* Frees all virtual memory */
	virtual void Reset();
private:
	LinearAllocator(LinearAllocator &linearAllocator);

protected:
	const std::size_t CalculatePadding(const std::size_t offset, const std::size_t alignment);

};

#endif /* LINEARALLOCATOR_H */
