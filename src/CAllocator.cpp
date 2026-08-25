#include "CAllocator.h"
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
    #include <malloc.h>

    #define PLATFORM_ALIGNED_ALLOC(ptr, align, size) (((ptr) = _aligned_malloc((size), (align))) == NULL ? -1 : 0)
    #define PLATFORM_ALIGNED_FREE(ptr) _aligned_free((ptr))
#else
    #include <cstdint>

    #define PLATFORM_ALIGNED_ALLOC(ptr, align, size) posix_memalign(&(ptr), (align), (size))
    #define PLATFORM_ALIGNED_FREE(ptr) std::free((ptr))
#endif

CAllocator::CAllocator() noexcept : Allocator(0), m_lastWasAligned(false) {}

CAllocator::~CAllocator() noexcept {}

void CAllocator::Init() {}

void* CAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
    if (alignment > 1) {
        void* ptr = NULL;
        const std::size_t align = (alignment < sizeof(void*)) ? sizeof(void*) : alignment;
        const std::size_t aligned_size = (size + align - 1) & ~(align - 1);

        if (PLATFORM_ALIGNED_ALLOC(ptr, align, aligned_size) != 0)
            return NULL;

        m_lastWasAligned = true;

        return ptr;
    }

    m_lastWasAligned = false;

    return std::malloc(size);
}

void CAllocator::Free(void* ptr) {
    if (m_lastWasAligned)
        PLATFORM_ALIGNED_FREE(ptr);
    else
        std::free(ptr);
}