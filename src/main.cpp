#include <iostream>
#include <cstddef>
#include "LinearAllocator.h"

struct bar {
	int a;	// 4
	bool b;	// 1 -> 5
	int c;	// 4 -> 12
	bool d;
	bool e;		// 3 -> 8

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

struct foo2 {
	char c;      /* 1 byte */
    char *p;     /* 8 bytes */
};

struct baz {
    short s;     /* 2 bytes */
    char c;      /* 1 byte */
};

/*
In general, a struct instance will have the alignment of its widest scalar member. 
Compilers do this as the easiest way to ensure that all the members are self-aligned for fast access.
*/

int main(){
	LinearAllocator linearAllocator(100);
	//linearAllocator.Allocate(sizeof(bar));
	/*
	linearAllocator.Allocate(sizeof(int));
	linearAllocator.Allocate(sizeof(bool));
	linearAllocator.Allocate(sizeof(int));
	linearAllocator.Allocate(sizeof(double));
	linearAllocator.Allocate(sizeof(char));
	linearAllocator.Allocate(sizeof(int));
*/
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




	return 1;

}