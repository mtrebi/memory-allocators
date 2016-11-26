#ifndef LINEARALLOCATOR_H
#define LINEARALLOCATOR_H

#include "Allocator.h"

class LinearAllocator : public Allocator {
protected:
	void* m_start_ptr;
	std::size_t m_offset;
public:
	LinearAllocator(const std::size_t totalSize);

	virtual ~LinearAllocator();

	virtual void* Allocate(const std::size_t size, const short alignment = 0) override;
	
	virtual void Free(void* ptr) override;

	virtual void Reset();
private:
	LinearAllocator(LinearAllocator &linearAllocator);

protected:
	const std::size_t CalculatePadding(const std::size_t offset, const std::size_t alignment);

};

#endif /* LINEARALLOCATOR_H */
