#include <typeinfo>
#include <cstdlib>
#include <stdint.h>

class LinearAllocator {
private:
	uint32_t m_totalSize;
	uint32_t m_offset;
public:
	/* Allocation of real memory */
	LinearAllocator(const uint32_t totalSize);

	/* Allocate virtual memory */
	void * allocate(const std::size_t allocationSize);

	/* Frees all virtual memory */
	void reset()

	/* Free all memory */
	~LinearAllocator();
};