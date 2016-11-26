#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstddef> // size_t


class Allocator {
protected:
	std::size_t m_totalSize;
    std::size_t m_used;
public:
	Allocator(const std::size_t totalSize);

	virtual ~Allocator();

	virtual void* Allocate(const std::size_t size, const short alignment = 0) = 0;

	virtual void Free(void* ptr) = 0;

	virtual void Init() = 0;
};

#endif /* ALLOCATOR_H */

