#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

#include "Allocator.h"

class FreeListAllocator : public Allocator {
public:
	enum PlacementPolicy {
		FIND_FIRST, 
		FIND_BEST;
	}

	enum StoragePolicy {
		LIFO,
		SORTED;
	}

private:
	void* m_start_ptr;
	PlacementPolicy m_pPolicy;
	StoragePolicy m_sPolicy;
	
	struct FreeBlock {
		FreeBlock * previous
					next;
		std::size_t size;
	} * m_freeList;



public:
	FreeListAllocator(const std::size_t totalSize, enum PlacementPolicy pPolicy, enum sPolicy);

	virtual ~FreeListAllocator();

	virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;
	
	virtual void Free(void* ptr) override;

	virtual void Init() override;

	virtual void Reset();
private:
	FreeListAllocator(FreeListAllocator &freeListAllocator);
	

	void InsertFree(void * ptr);
	void InsertFreeLIFO(void * ptr);
	void InsertFreeSorted(void * ptr);

	FreeBlock * Find(const std::size_t size);
	FreeBlock * FindBest(const std::size_t size);
	FreeBlock * FindFirst(const std::size_t size);
};

#endif /* FREELISTALLOCATOR_H */

