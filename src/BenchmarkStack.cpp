#include "BenchmarkStack.h"
#include "StackAllocator.h"
#include <iostream>		// cout cin ...
#include <stdlib.h>     /* srand, rand */

BenchmarkStack::BenchmarkStack(const int runtime)
	: Benchmark(runtime) {
}

BenchmarkResults BenchmarkStack::allocation() {
	std::cout << "STACK ALLOCATION" << std::endl;
	setStartTimer();
	
	StackAllocator stackAllocator(1e10);

	int operations = 0;
	while(!outOfTime()){
		stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
		stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
																	// 3  -> 8
		stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
		
		++operations;
	}

	BenchmarkResults results = buildResults(operations, m_runtime, stackAllocator.m_offset, (3+8)*operations);
	
	printResults(results);
	stackAllocator.Reset();
	return results;
}


BenchmarkResults BenchmarkStack::freeing() {
	std::cout << "STACK FREEING" << std::endl;

	timespec before, after;
	setTimer(before);
	StackAllocator stackAllocator(1e10);
	setTimer(after);
	double elapsedTime = calculateElapsedTime(before, after);

	std::size_t operations = 0;
	while(elapsedTime < (m_runtime * 1e3)){
		if (operations % 2 == 0){
			stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
			stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
																	// 3  -> 8
			stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
		}else {
			setTimer(before);
			stackAllocator.Free(sizeof(foo));
			stackAllocator.Free(sizeof(bool));
			stackAllocator.Free(sizeof(int));
			setTimer(after);
			elapsedTime += calculateElapsedTime(before, after);
		}
		++operations;
	}

	BenchmarkResults results = buildResults(operations/2, m_runtime, 0, 0);
	
	printResults(results);
	stackAllocator.Reset();
	return results;}

BenchmarkResults BenchmarkStack::read() {
	std::cout << "STACK READ" << std::endl;
	
	timespec before, after;
	setTimer(before);
	StackAllocator stackAllocator(1e10);
	setTimer(after);
	double elapsedTime = calculateElapsedTime(before, after);

	std::size_t operations = 0;
	while(elapsedTime < (m_runtime * 1e3)){
		int * i = (int *) stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
		bool * b = (bool *) stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
																	// 3  -> 8
		foo * f = (foo *) stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
		
		setTimer(before);
		int i_value = *i;
		bool b_value = *b;
		foo f_value = *f;
		setTimer(after);

		elapsedTime += calculateElapsedTime(before, after);
		
		stackAllocator.Free(sizeof(foo));
		stackAllocator.Free(sizeof(bool));
		stackAllocator.Free(sizeof(int));

		++operations;
	}

	BenchmarkResults results = buildResults(operations, m_runtime, 0, 0);
	
	printResults(results);
	stackAllocator.Reset();
	return results;
}

BenchmarkResults BenchmarkStack::write() {
	std::cout << "STACK WRITE" << std::endl;
	srand (0);


	timespec before, after;
	setTimer(before);
	StackAllocator stackAllocator(1e10);
	setTimer(after);
	double elapsedTime = calculateElapsedTime(before, after);

	std::size_t operations = 0;
	while(elapsedTime < (m_runtime * 1e3)){
		int * i = (int *) stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
		bool * b = (bool *) stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
																	// 3  -> 8
		foo * f = (foo *) stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
		
		int randomN = rand();
		setTimer(before);
		*i =  randomN % 100;
		*b = randomN % 2;
		*f = foo();
		setTimer(after);

		elapsedTime += calculateElapsedTime(before, after);

		stackAllocator.Free(sizeof(foo));
		stackAllocator.Free(sizeof(bool));
		stackAllocator.Free(sizeof(int));	

		++operations;
	}

	BenchmarkResults results = buildResults(operations, m_runtime, 0, 0);
	
	printResults(results);
	stackAllocator.Reset();
	return results;
}

BenchmarkResults BenchmarkStack::all() {
	std::cout << "STACK ALL" << std::endl;
	setStartTimer();
	StackAllocator stackAllocator(1e10);

	std::size_t operations = 0;
	int * i;
	bool * b;
	foo * f;
	while(!outOfTime()){
		if (operations % 2 == 0){
			i = (int *) stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
			b = (bool *) stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5													// 3  -> 8
			f = (foo *) stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
		}else {
			*i = *i + 1;
			*b = !b;
			*f = foo();

			stackAllocator.Free(sizeof(foo));
			stackAllocator.Free(sizeof(bool));
			stackAllocator.Free(sizeof(int));
		}

		++operations;
	}

	BenchmarkResults results = buildResults(operations, m_runtime, 0, 0);
	
	printResults(results);
	stackAllocator.Reset();
	return results;}