#include <iostream>
#include <cstddef>
#include <time.h>

#include "StackAllocator.h"

/*
void test_primitives(Allocator &allocator){
	std::cout << "\tTEST_PRIMITIVES_TYPES" << std::endl;

	allocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
	allocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
															// 3  -> 8
	allocator.Allocate(sizeof(int), alignof(int));			// 4  -> 12
															// 4  -> 16
	allocator.Allocate(sizeof(double), alignof(double));	// 8  -> 24
	allocator.Allocate(sizeof(char), alignof(char));		// 1  -> 25
															// 3  -> 28
	allocator.Allocate(sizeof(int), alignof(int));			// 4  -> 32
	
	allocator.Reset();
	std::cout << std::endl;

}

void test_primitives_unaligned(Allocator &allocator){
	std::cout << "\tTEST_PRIMITIVES_TYPES_UNALIGNED" << std::endl;

	allocator.Allocate(sizeof(int));		// 4  -> 4
	allocator.Allocate(sizeof(bool));		// 1  -> 5
											// 0  -> 5
	allocator.Allocate(sizeof(int));		// 4  -> 9
	allocator.Allocate(sizeof(double));		// 8  -> 17
	allocator.Allocate(sizeof(char));		// 1  -> 18
											// 0  -> 18
	allocator.Allocate(sizeof(int));		// 4  -> 22
	
	allocator.Reset();
	std::cout << std::endl;
}

void test_structs(Allocator &allocator){
	std::cout << "\tTEST_CUSTOM_TYPES" << std::endl;

	allocator.Allocate(sizeof(bar), alignof(bar));			// 16  -> 16
	allocator.Allocate(sizeof(foo), alignof(foo));			// 16  -> 32
	allocator.Allocate(sizeof(baz), alignof(baz));			// 4   -> 36
	allocator.Allocate(sizeof(bool), alignof(bool));		// 1   -> 37
															// 3   -> 40
	allocator.Allocate(sizeof(foo3), alignof(foo3));		// 8   -> 48
	allocator.Reset();
	std::cout << std::endl;
}

void test_structs_unaligned(Allocator &allocator){
	std::cout << "\tTEST_CUSTOM_TYPES_UNALIGNED" << std::endl;

	allocator.Allocate(sizeof(bar), 0);			// 16  -> 16
	allocator.Allocate(sizeof(foo), 0);			// 16  -> 32
	allocator.Allocate(sizeof(baz), 0);			// 4   -> 36
	allocator.Allocate(sizeof(bool), 0);		// 1   -> 37
												// 0   -> 37
	allocator.Allocate(sizeof(foo3), 0);		// 8   -> 45
	allocator.Reset();
	std::cout << std::endl;
}

void test_linear_allocator(){
	std::cout << "TEST_LINEAR_ALLOCATOR" << std::endl;

	LinearAllocator linearAllocator(100);
	test_primitives(linearAllocator);
	test_structs(linearAllocator);

	test_primitives_unaligned(linearAllocator);
	test_structs_unaligned(linearAllocator);
}

void test_stack_allocator_primitives(StackAllocator &stackAllocator){
	std::cout << "\tTEST_PRIMITIVES_TYPES" << std::endl;

	stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 1
																// 3  -> 4
	stackAllocator.Allocate(sizeof(baz), alignof(baz));			// 4  -> 8
	stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 12

	std::cout << std::endl;
	stackAllocator.Free(nullptr, sizeof(int));					// 4  -> 8
	stackAllocator.Free(nullptr, sizeof(baz));					// 8  -> 4(3) -> 1
																// 7  -> 8
	stackAllocator.Allocate(sizeof(double), alignof(double));	// 8  -> 16

	stackAllocator.Reset();
}

void test_stack_allocator(){
	std::cout << "TEST_STACK_ALLOCATOR" << std::endl;

	StackAllocator stackAllocator(100);
	//test_primitives(stackAllocator);
	//test_structs(stackAllocator);

	//test_primitives_unaligned(stackAllocator);
	//test_structs_unaligned(stackAllocator);
	test_stack_allocator_primitives(stackAllocator);
}
*/

#include "BenchmarkStack.h"
#include "BenchmarkC.h"
int main(){
	BenchmarkStack benchmarkStack(1);
	//benchmarkStack.allocation();
	//benchmarkStack.freeing();
	//benchmarkStack.read();
	//benchmarkStack.write();
	//benchmarkStack.all();

	BenchmarkC benchmarkC(1);
	//benchmarkC.allocation();
	//benchmarkC.freeing();
	//benchmarkC.read();
	//benchmarkC.write();
	//benchmarkC.all();

	return 1;
}




