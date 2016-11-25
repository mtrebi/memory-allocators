#include "BenchmarkStack.h"
#include "StackAllocator.h"
#include <iostream>

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

	BenchmarkResults results = buildResults(operations, m_runtime, 0, 0);
	
	printResults(results);
	stackAllocator.Reset();
	return results;
}


BenchmarkResults BenchmarkStack::freeing() {
	std::cout << "STACK FREEING" << std::endl;

	setStartTimer();
	
	StackAllocator stackAllocator(1e10);

	std::size_t operations = 0;
	double elapsedTime = 0;
	while(elapsedTime < (m_runtime * 1e3)){
		if (operations % 2 == 0){
			stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
			stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
																	// 3  -> 8
			stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
		}else {
			timespec before_free, after_free;
			setTimer(before_free);
			stackAllocator.Free(sizeof(foo));
			stackAllocator.Free(sizeof(bool));
			stackAllocator.Free(sizeof(int));
			setTimer(after_free);
			elapsedTime += calculateElapsedTime(before_free, after_free);
		}
		++operations;
	}

	BenchmarkResults results = buildResults(operations/2, m_runtime, 0, 0);
	
	printResults(results);
	stackAllocator.Reset();
	return results;}

BenchmarkResults BenchmarkStack::read() {
	return buildResults(0, 0, 0, 0);
}

BenchmarkResults BenchmarkStack::write() {
	return buildResults(0, 0, 0, 0);
}

BenchmarkResults BenchmarkStack::all() {
	return buildResults(0, 0, 0, 0);
}