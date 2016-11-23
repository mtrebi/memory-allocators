#include <iostream>
#include <cstddef>
#include "LinearAllocator.h"
#include "Allocator.h"
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

struct foo {
    char *p;     /* 8 bytes */
    char c;      /* 1 byte */
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

int main(){
	test_linear_allocator();
	return 1;
}

	/*
	std::cout << "bar" << std::endl;
	std::cout << "\tsizeof() = " << sizeof(bar) << std::endl;
	std::cout << "\talignf() = " << alignof(bar) << std::endl;
	std::cout << "\toffset(a) =" << offsetof(bar, a) << std::endl;
	std::cout << "\toffset(b) =" << offsetof(bar, b) << std::endl;
	std::cout << "\toffset(c) =" << offsetof(bar, c) << std::endl;


	std::cout << "foo" << std::endl;
	std::cout << "\tsizeof() = " << sizeof(foo) << std::endl;
	std::cout << "\talignf() = " << alignof(foo) << std::endl;
	std::cout << "\toffset(c) =" << offsetof(foo, c) << std::endl;
	std::cout << "\toffset(p) =" << offsetof(foo, p) << std::endl;

	std::cout << "foo2" << std::endl;
	std::cout << "\tsizeof() = " << sizeof(foo2) << std::endl;
	std::cout << "\talignf() = " << alignof(foo2) << std::endl;
	std::cout << "\toffset(c) =" << offsetof(foo2, c) << std::endl;
	std::cout << "\toffset(p) =" << offsetof(foo2, p) << std::endl;


	std::cout << "baz" << std::endl;
	std::cout << "\tsizeof() = " << sizeof(baz) << std::endl;
	std::cout << "\talignf() = " << alignof(baz) << std::endl;
	std::cout << "\toffset(s) =" << offsetof(baz, s) << std::endl;
	std::cout << "\toffset(c) =" << offsetof(baz, c) << std::endl;
*/