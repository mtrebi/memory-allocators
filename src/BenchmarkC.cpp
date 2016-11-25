#include "BenchmarkC.h"
#include <stdlib.h>     /* malloc, calloc, realloc, free */
#include <cstddef> // std::size_t
#include <iostream>

BenchmarkC::BenchmarkC(const int runtime)
	: Benchmark(runtime) {
}

BenchmarkResults BenchmarkC::allocation() {
	std::cout << "C ALLOCATION" << std::endl;

	setStartTimer();

	int operations = 0;
	while(!outOfTime()){
		malloc(sizeof(int));
		malloc(sizeof(bool));
		malloc(sizeof(foo));		

		++operations;
	}

	BenchmarkResults results = buildResults(operations, m_runtime, 16*3*operations, (12+15+8)*operations);
	
	printResults(results);

	return results;
}

/*	Theres no way to benchmark only free calls.
	One option could be allocate and store all pointers in a stack but then the benchmarking would be biased
	The onyl thing we can do is allocate memory and then free it afterwards.
*/
BenchmarkResults BenchmarkC::freeing() {
	std::cout << "C FREEING" << std::endl;

	setStartTimer();
	int * i;
	bool * b;
	foo * f;
	std::size_t operations = 0;
	double elapsedTime = 0;
	while(elapsedTime < (m_runtime * 1e3)){
		if (operations % 2 == 0){
			i = (int*) malloc(sizeof(int));
			b = (bool*)malloc(sizeof(bool));
			f = (foo*) malloc(sizeof(foo));	

		}else {
			timespec before_free, after_free;
			setTimer(before_free);
			free(f);
			free(b);
			free(i);
			setTimer(after_free);
			elapsedTime += calculateElapsedTime(before_free, after_free);
		}
		++operations;
	}

	BenchmarkResults results = buildResults(operations/2, m_runtime, 0, 0);
	
	printResults(results);

	return results;
}

BenchmarkResults BenchmarkC::read() {
	std::cout << "C READ" << std::endl;

	std::size_t operations = 0;
	double elapsedTime = 0;
	while(elapsedTime < (m_runtime * 1e3)){
		int * i = (int*) malloc(sizeof(int));
		bool * b = (bool*)malloc(sizeof(bool));
		foo * f = (foo*) malloc(sizeof(foo));	

		timespec before_read, after_read;
		setTimer(before_read);
		int i_value = *i;
		bool b_value = *b;
		foo f_value = *f;
		setTimer(after_read);

		free(f);
		free(b);
		free(i);

		elapsedTime += calculateElapsedTime(before_read, after_read);
		++operations;
	}

	BenchmarkResults results = buildResults(operations, m_runtime, 0, 0);
	
	printResults(results);

	return results;
}

BenchmarkResults BenchmarkC::write() {
	std::cout << "C WRITE" << std::endl;
	srand (0);

	std::size_t operations = 0;
	double elapsedTime = 0;
	while(elapsedTime < (m_runtime * 1e3)){
		int * i = (int*) malloc(sizeof(int));
		bool * b = (bool*)malloc(sizeof(bool));
		foo * f = (foo*) malloc(sizeof(foo));	

		int randomN = rand();
		timespec before_write, after_write;
		setTimer(before_write);
		*i =  randomN % 100;
		*b = randomN % 2;
		*f = foo();
		setTimer(after_write);

		free(f);
		free(b);
		free(i);


		elapsedTime += calculateElapsedTime(before_write, after_write);

		++operations;
	}

	BenchmarkResults results = buildResults(operations, m_runtime, 0, 0);
	
	printResults(results);

	return results;}

BenchmarkResults BenchmarkC::all() {
	std::cout << "C ALL" << std::endl;

	setStartTimer();
	int * i;
	bool * b;
	foo * f;
	std::size_t operations = 0;
	double elapsedTime = 0;
	while(!outOfTime()){
		if (operations % 5 < 3){
			i = (int*) malloc(sizeof(int));
			b = (bool*)malloc(sizeof(bool));
			f = (foo*) malloc(sizeof(foo));	
		}else {
			*i = *i + 1;
			*b = !b;
			*f = foo();

			free(f);
			free(b);
			free(i);
		}
		++operations;
	}

	BenchmarkResults results = buildResults(operations, m_runtime, 0, 0);
	
	printResults(results);

	return results;



}