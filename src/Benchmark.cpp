#include "Benchmark.h"
#include <iostream>

Benchmark::Benchmark(long nOperations){
	m_nOperations = nOperations;
}

void Benchmark::setStartTimer(){
	setTimer(m_start);
}

void Benchmark::setEndTimer(){
	setTimer(m_end);
}

void Benchmark::setTimer(timespec& timer){
  clock_gettime(CLOCK_REALTIME, &timer);
}

const timespec Benchmark::calculateElapsedTime() const {
    timespec temp;
    if ((m_end.tv_nsec-m_start.tv_nsec)<0) {
        temp.tv_sec = m_end.tv_sec-m_start.tv_sec-1;
        temp.tv_nsec = 1e9+m_end.tv_nsec-m_start.tv_nsec;
    } else {
        temp.tv_sec = m_end.tv_sec-m_start.tv_sec;
        temp.tv_nsec = m_end.tv_nsec-m_start.tv_nsec;
    }
    return temp;
}

void Benchmark::printResults(const BenchmarkResults& results) const {
	std::cout << std::endl;
	std::cout << "\tRESULTS:" << std::endl;
	std::cout << "\t\tOperations:    \t" << results.nOperations  << std::endl;
	std::cout << "\t\tTime elapsed:  \t" << results.elapsedTime << " ms" << std::endl;
	std::cout << "\t\tOp per sec:    \t" << results.operationsPerSec / 1e3 << " mops/ms" << std::endl;
	std::cout << "\t\tTimer per op:  \t" << results.timePerOperation / 1e3 << " ms/mops" << std::endl;
	std::cout << "\t\tMemory used:   \t" << results.memoryWasted  << " bytes" << std::endl;
	std::cout << "\t\tMemory wasted: \t" << results.memoryUsed  << " bytes\t" << std::endl;
	std::cout << std::endl;
}

const BenchmarkResults Benchmark::buildResults(const long nOperations, const timespec elapsedTime, const std::size_t memoryUsed, const std::size_t memoryWasted) const{
	BenchmarkResults results;
	results.nOperations = nOperations;

	double time_sec = (double) elapsedTime.tv_sec;
  	double time_nsec = (double) elapsedTime.tv_nsec;
  	double time_msec = (time_sec * 1000) + (time_nsec / 1000000);

  	results.elapsedTime = time_msec;
  	results.operationsPerSec = results.nOperations / results.elapsedTime;
  	results.timePerOperation = results.elapsedTime / results.nOperations;
  	results.memoryUsed = memoryUsed;
  	results.memoryWasted = memoryWasted;

  	return results;
}