#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

#include "Allocator.h"
#include "DoublyLinkedList.h"

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

	struct FreeBlockHeader {
		std::size_t blockSize;
	};

	struct AllocatedBlockHeader {
		std::size_t blockSize;
	};


private:
	void* m_start_ptr;
	PlacementPolicy m_pPolicy;
	StoragePolicy m_sPolicy;
	DoublyLinkedList<FreeBlockHeader> * m_freeList;



public:
	FreeListAllocator(const std::size_t totalSize, enum PlacementPolicy pPolicy, enum sPolicy);

	virtual ~FreeListAllocator();

	virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;
	
	virtual void Free(void* ptr) override;

	virtual void Init() override;

	virtual void Reset();
private:
	FreeListAllocator(FreeListAllocator &freeListAllocator);
	
	void Coalescence(Node<FreeBlockHeader> * freeBlock);

	Node<FreeBlockHeader> * InsertFree(void * ptr);
	Node<FreeBlockHeader> * InsertFreeLIFO(void * ptr);
	Node<FreeBlockHeader> * InsertFreeSorted(void * ptr);

	Node<FreeBlockHeader> * Find(const std::size_t size);
	Node<FreeBlockHeader> * FindBest(const std::size_t size);
	Node<FreeBlockHeader> * FindFirst(const std::size_t size);
};

#endif /* FREELISTALLOCATOR_H */

