#include <iostream>
#include <cstddef>
#include <vector>

#include "Benchmark.h"
#include "Allocator.h"
#include "StackAllocator.h"
#include "CAllocator.h"
#include "LinearAllocator.h"

int main(){
	const std::vector<std::size_t> ALLOCATION_SIZES {16, 32, 64, 256, 512, 1024};
	const std::size_t ALIGNMENT = 8;

	Allocator * cAllocator = new CAllocator();
	Allocator * linearAllocator = new LinearAllocator(1e5);
	Allocator * stackAllocator = new StackAllocator(1e5);

	Benchmark benchmark(2, ALLOCATION_SIZES, ALIGNMENT);
	
	std::cout << "C" << std::endl;
	benchmark.Allocation(cAllocator);
	benchmark.Free(cAllocator);
	std::cout << "LINEAR" << std::endl;
	benchmark.Allocation(linearAllocator);

	std::cout << "STACK" << std::endl;
	benchmark.Allocation(stackAllocator);
	benchmark.Free(stackAllocator);



	delete cAllocator;
	delete linearAllocator;
	delete stackAllocator;
	
	return 1;
}




