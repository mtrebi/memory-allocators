#ifndef CALLOCATOR_H
#define CALLOCATOR_H

#include "Allocator.h"

class CAllocator : public Allocator {
public:
	CAllocator();

	virtual ~CAllocator();

	virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;

	virtual void Free(void* ptr) override;

	virtual void Init() override;
};

#endif /* CALLOCATOR_H */

