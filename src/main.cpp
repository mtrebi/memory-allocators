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



struct foo {
    char *p;     /* 8 bytes */
    char c;      /* 1 byte */
};

struct bar {
	int a;		// 4
	bool b;		// 1 -> 5
				// 3 -> 8
	int c;		// 4 -> 12
	bool d;		// 1 -> 13
	bool e;		// 1 -> 14
				// 2 -> 16
};

struct bar2 {
	int a;	// 4
	int c;	// 4 -> 8
	bool b;	// 1 -> 9
	bool d;
	bool e;
			// 3 -> 12
};

struct foo3 {
	int i;      /* 4 byte */
    char c;     /* 1 bytes */
    bool b;		/* 1 bytes */
    			// 2 bytes
};

struct foo2 {
	char c;      /* 1 byte */
    char *p;     /* 8 bytes */
};

struct baz {
    short s;     /* 2 bytes */
    char c;      /* 1 byte */
};


void setTimer(timespec& timer){
  clock_gettime(CLOCK_REALTIME, &timer);
}

timespec diff(timespec &start, timespec &end) {
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1e9+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}


void print_benchmark_stats(const timespec& elapsed_time, const int& memory_used, const int&memory_wasted, const int max_operations){
	double time_sec = (double) elapsed_time.tv_sec;
  	double time_nsec = (double) elapsed_time.tv_nsec;
  	double time_msec = (time_sec * 1000) + (time_nsec / 1000000);

	std::cout << std::endl;
	std::cout << "\tSTATS:" << std::endl;
	std::cout << "\t\tOperations:    \t" << max_operations  << std::endl;
	std::cout << "\t\tTime elapsed:  \t" << time_msec << " ms" << std::endl;
	std::cout << "\t\tOp per sec:    \t" << (max_operations / 1e3) / time_msec << " mops/ms" << std::endl;

	//std::cout << "\t\tMemory used:   \t" << memory_used  << " bytes" << std::endl;
	//std::cout << "\t\tMemory wasted: \t" << memory_wasted  << " bytes\t" << ((float) memory_wasted / memory_used) * 100 << " %" << std::endl;
	std::cout << std::endl;
}


void benchmark_stack(long MAX_OPERATIONS = 1e4){
	timespec start, end;

	std::cout << "BENCHMARK STACK ALLOCATOR: START" << std::endl;

	setTimer(start);
	
	StackAllocator stackAllocator(1e10);

	int operations = 0;
	while(operations < MAX_OPERATIONS){
		stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
		stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
																	// 3  -> 8
		stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
		++operations;
	}
	setTimer(end); 
	const timespec elapsed_time = diff(start, end);
	const std::size_t memory_used = 0;
	const std::size_t  memory_wasted = 0;
	stackAllocator.Reset();
	print_benchmark_stats(elapsed_time, memory_used, memory_wasted, MAX_OPERATIONS);
	std::cout << "BENCHMARK STACK ALLOCATOR: END" << std::endl;
}

void benchmark_malloc(long MAX_OPERATIONS = 1e4){
	timespec start, end;

	std::cout << "BENCHMARK MALLOC ALLOCATOR: START" << std::endl;

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
	std::cout << "BENCHMARK MALLOC ALLOCATOR: END" << std::endl;
}

int main(){
	benchmark_stack(1e7);
	//benchmark_malloc(1e7);
	return 1;
}




