#include "Allocator.h"


class LinearAllocator : public Allocator {
private:
	/* Offset from the start of the memory block */
	uint32_t m_offset;
public:
	/* Allocation of real memory */
	LinearAllocator(const uint32_t totalSize);

	/* Frees all memory */
	virtual ~LinearAllocator();

	/* Init method */
	virtual void Init() override;

	/* Allocate virtual memory */
	virtual void* Allocate(const std::size_t allocationSize) override;

	/* Frees virtual memory */
	virtual void Free(void* ptr) override;

	/* Frees all virtual memory */
	virtual void Reset() override;
};