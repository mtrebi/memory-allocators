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







/*

void benchmark_stack_allocate_free(long MAX_OPERATIONS = 1e4){
	timespec start, end;

	std::cout << "BENCHMARK STACK A/F: START" << std::endl;

	setTimer(start);
	
	StackAllocator stackAllocator(1e10);

	int operations = 0;
	bool allocate = true;
	int * i;
	bool * b;
	foo * f;
	while(operations < MAX_OPERATIONS){
		if (allocate) {
			i = (int*) stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
			b = (bool*) stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
																					// 3  -> 8
			f = (foo*) stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
			allocate = false;
		}else {
			stackAllocator.Free(sizeof(foo));
			stackAllocator.Free(sizeof(bool));
			stackAllocator.Free(sizeof(int));
			allocate = true;
		}
		++operations;
	}
	setTimer(end); 
	const timespec elapsed_time = diff(start, end);
	const std::size_t memory_used = 0;
	const std::size_t  memory_wasted = 0;
	stackAllocator.Reset();
	print_benchmark_stats(elapsed_time, memory_used, memory_wasted, MAX_OPERATIONS);
	std::cout << "BENCHMARK STACK A/F: END" << std::endl;
}

void benchmark_stack_allocate_free_read_write(long MAX_OPERATIONS = 1e4){
	timespec start, end;

	std::cout << "BENCHMARK STACK A/F/R/W: START" << std::endl;

	setTimer(start);
	
	StackAllocator stackAllocator(1e10);

	int operations = 0;
	bool allocate = true;
	int * i;
	bool * b;
	foo * f;
	while(operations < MAX_OPERATIONS){
		if (allocate) {
			i = (int*) stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
			b = (bool*) stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
																					// 3  -> 8
			f = (foo*) stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
			
			*i = *i + 1;
			*b = allocate;
			*f = foo();

			allocate = false;
		}else {
			stackAllocator.Free(sizeof(foo));
			stackAllocator.Free(sizeof(bool));
			stackAllocator.Free(sizeof(int));
			allocate = true;
		}
		++operations;
	}
	setTimer(end); 
	const timespec elapsed_time = diff(start, end);
	const std::size_t memory_used = 0;
	const std::size_t  memory_wasted = 0;
	stackAllocator.Reset();
	print_benchmark_stats(elapsed_time, memory_used, memory_wasted, MAX_OPERATIONS);
	std::cout << "BENCHMARK STACK A/F/R/W: END" << std::endl;
}



void benchmark_malloc_allocate(long MAX_OPERATIONS = 1e4){
	timespec start, end;

	std::cout << "BENCHMARK MALLOC A: START" << std::endl;

	setTimer(start);
	
	int operations = 0;
	srand (1);

	while(operations < MAX_OPERATIONS){
		malloc(sizeof(int));
		malloc(sizeof(bool));
		malloc(sizeof(foo));
		++operations;
	}

	setTimer(end); 
	const timespec elapsed_time = diff(start, end);
	const std::size_t memory_used = 0;
	const std::size_t  memory_wasted = 0;

	print_benchmark_stats(elapsed_time, memory_used, memory_wasted, MAX_OPERATIONS);
	std::cout << "BENCHMARK MALLOC A: END" << std::endl;
}

void benchmark_malloc_allocate_free(long MAX_OPERATIONS = 1e4){
	timespec start, end;

	std::cout << "BENCHMARK MALLOC A/F: START" << std::endl;

	setTimer(start);
	
	int operations = 0;
	bool allocate = true;
	int * i;
	bool * b;
	foo * f;
	while(operations < MAX_OPERATIONS){
		if (allocate){
			i = (int*) malloc(sizeof(int));
			b = (bool*) malloc(sizeof(bool));
			f = (foo*) malloc(sizeof(foo));
			allocate = false;	
		}else {
			free(f);
			free(b);
			free(i);
			allocate = true;
		}

		++operations;
	}

	setTimer(end); 
	const timespec elapsed_time = diff(start, end);
	const std::size_t memory_used = 0;
	const std::size_t  memory_wasted = 0;

	print_benchmark_stats(elapsed_time, memory_used, memory_wasted, MAX_OPERATIONS);
	std::cout << "BENCHMARK MALLOC A/F: END" << std::endl;
}

void benchmark_malloc_allocate_free_read_write(long MAX_OPERATIONS = 1e4){
	timespec start, end;

	std::cout << "BENCHMARK MALLOC A/F/R/W: START" << std::endl;

	setTimer(start);
	
	int operations = 0;
	bool allocate = true;
	int * i;
	bool * b;
	foo * f;
	while(operations < MAX_OPERATIONS){
		if (allocate){
			i = (int*) malloc(sizeof(int));
			b = (bool*) malloc(sizeof(bool));
			f = (foo*) malloc(sizeof(foo));

			*i = *i + 1;
			*b = allocate;
			*f = foo();

			allocate = false;	
		}else {
			free(f);
			free(b);
			free(i);
			allocate = true;
		}

		++operations;
	}

	setTimer(end); 
	const timespec elapsed_time = diff(start, end);
	const std::size_t memory_used = 0;
	const std::size_t  memory_wasted = 0;

	print_benchmark_stats(elapsed_time, memory_used, memory_wasted, MAX_OPERATIONS);
	std::cout << "BENCHMARK MALLOC A/F/R/W: END" << std::endl;
}
*/
/* TODO
	2- Stack/Linear ->Calculate padding (Aligned allocators interface?)
	3- test linear too
*/

#include "BenchmarkStack.h"

int main(){
	BenchmarkStack benchmarkStack(1e3);
	benchmarkStack.allocation();
	//benchmarkStack.free();
	//benchmarkStack.read();
	//benchmarkStack.write();
	//benchmarkStack.all();
	return 1;
}




