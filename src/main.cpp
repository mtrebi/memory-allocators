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

// D-15: 原实现依赖 Benchmark.h 里 "OPERATIONS" 宏（硬编码 10），
// 既污染全局命名空间，又与 Benchmark 构造参数语义重复。
// 改为显式常量。
static const unsigned int kOperations = 1000;

int main()
{
    const std::size_t A = static_cast<std::size_t>(1e9);
    const std::size_t B = static_cast<std::size_t>(1e8);

    const std::vector<std::size_t> ALLOCATION_SIZES {32, 64, 256, 512, 1024, 2048, 4096};
    const std::vector<std::size_t> ALIGNMENTS {8, 8, 8, 8, 8, 8, 8};

    Allocator * cAllocator = new CAllocator();
    Allocator * linearAllocator = new LinearAllocator(A);
    Allocator * stackAllocator = new StackAllocator(A);
    // 池分配器的 chunkSize 会被向上对齐到 alignof(std::max_align_t)
    Allocator * poolAllocator = new PoolAllocator(16777216, 4096);
    Allocator * freeListAllocator = new FreeListAllocator(B, FreeListAllocator::PlacementPolicy::FIND_FIRST);

    Benchmark benchmark(kOperations);

    std::cout << "C" << std::endl;
    benchmark.MultipleAllocation(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.MultipleFree(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomFree(cAllocator, ALLOCATION_SIZES, ALIGNMENTS); 

    std::cout << "LINEAR" << std::endl;
    // 线性分配器不支持单次释放（IsFreeable() == false），只测分配
    benchmark.MultipleAllocation(linearAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(linearAllocator, ALLOCATION_SIZES, ALIGNMENTS);

    std::cout << "STACK" << std::endl;
    benchmark.MultipleAllocation(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.MultipleFree(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomFree(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);

    std::cout << "POOL" << std::endl;
    benchmark.SingleAllocation(poolAllocator, 4096, 8);
    benchmark.SingleFree(poolAllocator, 4096, 8);

    std::cout << "FREE LIST" << std::endl;
    benchmark.MultipleAllocation(freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.MultipleFree(freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomFree(freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    
    delete cAllocator;
    delete linearAllocator;
    delete stackAllocator;
    delete poolAllocator;
    // D-18: 原实现漏掉这一行。FreeListAllocator 析构函数会 free 内部
    // 申请的池内存，不 delete 就是一处确定的泄漏。
    delete freeListAllocator;
    
    return 0;
}
