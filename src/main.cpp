#include <iostream>

#include "LinearAllocator.h"
int main(){
	LinearAllocator linearAllocator(100);

	linearAllocator.Allocate(sizeof(int));
	linearAllocator.Allocate(sizeof(bool));
	linearAllocator.Allocate(sizeof(int));
	linearAllocator.Allocate(sizeof(double));

	return 1;

}