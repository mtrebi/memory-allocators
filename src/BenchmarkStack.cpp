#include "BenchmarkStack.h"
#include "StackAllocator.h"

BenchmarkStack::BenchmarkStack(long nOperations)
	: Benchmark(nOperations) {
}

BenchmarkResults BenchmarkStack::allocation() {
	setStartTimer();
	
	StackAllocator stackAllocator(1e10);

	int operations = 0;
	while(operations < m_nOperations){
		stackAllocator.Allocate(sizeof(int), alignof(int));			// 4  -> 4
		stackAllocator.Allocate(sizeof(bool), alignof(bool));		// 1  -> 5
																	// 3  -> 8
		stackAllocator.Allocate(sizeof(foo), alignof(foo));			// 16 -> 24
		
		++operations;
	}
	setEndTimer();

	BenchmarkResults results = buildResults(m_nOperations, calculateElapsedTime(), 0, 0);
	
	printResults(results);

	return results;
}


BenchmarkResults BenchmarkStack::free() {
	return buildResults(0, calculateElapsedTime(), 0, 0);
}

BenchmarkResults BenchmarkStack::read() {
	return buildResults(0, calculateElapsedTime(), 0, 0);
}

BenchmarkResults BenchmarkStack::write() {
	return buildResults(0, calculateElapsedTime(), 0, 0);
}

BenchmarkResults BenchmarkStack::all() {
	return buildResults(0, calculateElapsedTime(), 0, 0);
}