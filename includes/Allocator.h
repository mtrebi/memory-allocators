#include <typeinfo>
#include <stdint.h>

class Allocator {
protected:
	/* Memory size */
	long m_totalSize;
	
	/* Pointer to the start of the memory block. Once set, never changes */
	void* m_start_ptr;
public:
	/* Allocation of memory */
	Allocator(const long totalSize);

	/* Frees all memory */
	virtual ~Allocator();

	/* Allocate virtual memory */
	virtual void* Allocate(const std::size_t size, const std::size_t alignment) = 0;

	/* Frees virtual memory */
	virtual void Free(void* ptr) = 0;

	/* Frees all virtual memory */
	virtual void Reset() = 0;
};