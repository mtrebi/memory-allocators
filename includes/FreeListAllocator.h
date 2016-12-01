#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

#include "Allocator.h"
#include "DoublyLinkedList.h"

class FreeListAllocator : public Allocator {
public:
	enum PlacementPolicy {
		FIND_FIRST, 
		FIND_BEST
	};

	enum StoragePolicy {
		LIFO,
		SORTED
	};

	struct BlockHeader {
		std::size_t blockSize;
	};

private:
	void* m_start_ptr;
	PlacementPolicy m_pPolicy;
	StoragePolicy m_sPolicy;
	DoublyLinkedList<BlockHeader> m_freeList;

public:
	FreeListAllocator(const std::size_t totalSize, PlacementPolicy pPolicy, StoragePolicy sPolicy);

	virtual ~FreeListAllocator();

	virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;
	
	virtual void Free(void* ptr) override;

	virtual void Init() override;

	virtual void Reset();
private:
	FreeListAllocator(FreeListAllocator &freeListAllocator);
	
	void Coalescence(Node<BlockHeader> * freeBlock);

	Node<BlockHeader> * InsertFree(void * ptr);
	Node<BlockHeader> * InsertFreeLIFO(void * ptr);
	Node<BlockHeader> * InsertFreeSorted(void * ptr);

	Node<BlockHeader> * Find(const std::size_t size, const std::size_t alignment, std::size_t& padding);
	Node<BlockHeader> * FindBest(const std::size_t size, const std::size_t alignment, std::size_t& padding);
	Node<BlockHeader> * FindFirst(const std::size_t size, const std::size_t alignment, std::size_t& padding);
};

#endif /* FREELISTALLOCATOR_H */

