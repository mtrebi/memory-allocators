#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

#include "Allocator.h"

class FreeListAllocator : public Allocator {
private:
	void* m_start_ptr;

	struct FreeBlock {
		FreeBlock * previous
					next;
		std::size_t size;
	} * m_freeList;

	enum PlacementPolicy {
		FIND_FIRST, 
		FIND_BEST;
	} m_pPolicy;

public:
	FreeListAllocator(const std::size_t totalSize, enum PlacementPolicy policy);

	virtual ~FreeListAllocator();

	virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;
	
	virtual void Free(void* ptr) override;

	virtual void Init() override;

	virtual void Reset();
private:
	FreeListAllocator(FreeListAllocator &freeListAllocator);
	
	FreeBlock * FindBest(const std::size_t size);
	FreeBlock * FindFirst(const std::size_t size);
};

#endif /* FREELISTALLOCATOR_H */

