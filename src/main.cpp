#include <iostream>
#include <cstddef>
#include <vector>

#include "Benchmark.h"
#include "Allocator.h"
#include "StackAllocator.h"
#include "CAllocator.h"
#include "LinearAllocator.h"
#include "PoolAllocator.h"
#include "FreeListAllocator.h"

int main()
{
    const std::size_t GB = static_cast<std::size_t>(1e9);
    const std::size_t MB = static_cast<std::size_t>(1e8);
    const std::size_t POOL_SIZE = 16777216;
    const std::size_t CHUNK_SIZE = 4096;

    const std::size_t sizes_arr[] = { 32, 64, 256, 512, 1024, 2048, 4096 };
    const std::size_t alignments_arr[] = { 8, 8, 8, 8, 8, 8, 8 };

    const std::vector<std::size_t> ALLOCATION_SIZES(sizes_arr, sizes_arr + 7);
    const std::vector<std::size_t> ALIGNMENTS(alignments_arr, alignments_arr + 7);

    CAllocator cAllocator;
    LinearAllocator linearAllocator(GB);
    StackAllocator stackAllocator(GB);
    PoolAllocator poolAllocator(POOL_SIZE, CHUNK_SIZE);
    FreeListAllocator freeListAllocator(MB, FreeListAllocator::PlacementPolicy::FIND_FIRST);

    const unsigned int N_OPERATIONS = 1000;
    Benchmark benchmark(N_OPERATIONS);

    std::cout << "=== C ALLOCATOR ===" << IO::endl;
    benchmark.MultipleAllocation(&cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.MultipleFree(&cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(&cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomFree(&cAllocator, ALLOCATION_SIZES, ALIGNMENTS);

    std::cout << "=== LINEAR ALLOCATOR ===" << IO::endl;
    benchmark.MultipleAllocation(&linearAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(&linearAllocator, ALLOCATION_SIZES, ALIGNMENTS);

    std::cout << "=== STACK ALLOCATOR ===" << IO::endl;
    benchmark.MultipleAllocation(&stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.MultipleFree(&stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(&stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomFree(&stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);

    std::cout << "=== POOL ALLOCATOR ===" << IO::endl;
    benchmark.SingleAllocation(&poolAllocator, CHUNK_SIZE, 8);
    benchmark.SingleFree(&poolAllocator, CHUNK_SIZE, 8);

    std::cout << "=== FREE LIST ALLOCATOR ===" << IO::endl;
    benchmark.MultipleAllocation(&freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.MultipleFree(&freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(&freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomFree(&freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);

    return 0;
}