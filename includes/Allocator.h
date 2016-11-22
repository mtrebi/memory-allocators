#include <typeinfo>
#include <stdint.h>

class Allocator {
protected:
	/* Memory size */
	uint32_t m_totalSize;
	
	/* Pointer to the start of the memory block. Once set, never changes */
	void* m_start_ptr;
public:
	/* Allocation of memory */
	Allocator(const uint32_t totalSize);

	/* Frees all memory */
	virtual ~Allocator();

	/* Allocate virtual memory */
	virtual void* Allocate(const std::size_t allocationSize) = 0;

	/* Frees virtual memory */
	virtual void Free(void* ptr) = 0;

	/* Frees all virtual memory */
	virtual void Reset() = 0;

protected:
	/* Init method */
	virtual void Init() = 0;
};