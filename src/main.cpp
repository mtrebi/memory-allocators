#include <iostream>
#include <cstddef>
#include <vector>

#include "Benchmark.h"
#include "Allocator.h"
#include "StackAllocator.h"
#include "CAllocator.h"
#include "LinearAllocator.h"
#include "PoolAllocator.h"

int main(){
	
	const std::vector<std::size_t> ALLOCATION_SIZES {2, 4, 16, 32, 64, 256, 512, 1024};
	const std::vector<std::size_t> ALIGNMENTS {2, 4, 8, 8, 8, 8, 8, 8};

	Allocator * cAllocator = new CAllocator();
	Allocator * linearAllocator = new LinearAllocator(1e8);
	Allocator * stackAllocator = new StackAllocator(1e8);
	Allocator * poolAllocator = new PoolAllocator(4096, 8);

	Benchmark benchmark(100);
	
	std::cout << "C" << std::endl;
	benchmark.MultipleAllocation(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
	benchmark.MultipleFree(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
	benchmark.RandomAllocation(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
	benchmark.RandomFree(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);

	std::cout << "LINEAR" << std::endl;
	benchmark.MultipleAllocation(linearAllocator, ALLOCATION_SIZES, ALIGNMENTS);
	benchmark.RandomAllocation(linearAllocator, ALLOCATION_SIZES, ALIGNMENTS);

	std::cout << "STACK" << std::endl;
	benchmark.MultipleAllocation(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
	benchmark.MultipleFree(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
	benchmark.RandomAllocation(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
	benchmark.RandomFree(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);

	std::cout << "POOL" << std::endl;
	benchmark.SingleAllocation(poolAllocator, 8, 0);
	benchmark.SingleFree(poolAllocator, 8, 0);

	delete cAllocator;
	delete linearAllocator;
	delete stackAllocator;
	delete poolAllocator;
	
	return 1;
}




