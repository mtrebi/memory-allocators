#include "BenchmarkC.h"
#include <cstdlib> // malloc, free

BenchmarkC::BenchmarkC(const int runtime)
	: Benchmark(runtime) {
}

BenchmarkResults BenchmarkC::allocation() {
	setStartTimer();


	int operations = 0;
	while(!outOfTime()){
		malloc(sizeof(int));
		malloc(sizeof(bool));
		malloc(sizeof(foo));		

		++operations;
	}

	BenchmarkResults results = buildResults(operations, m_runtime, 0, 0);
	
	printResults(results);

	return results;
}


BenchmarkResults BenchmarkC::free() {
	return buildResults(0, 0, 0, 0);
}

BenchmarkResults BenchmarkC::read() {
	return buildResults(0, 0, 0, 0);
}

BenchmarkResults BenchmarkC::write() {
	return buildResults(0, 0, 0, 0);
}

BenchmarkResults BenchmarkC::all() {
	return buildResults(0, 0, 0, 0);
}