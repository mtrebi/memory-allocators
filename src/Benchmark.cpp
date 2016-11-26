#include "Benchmark.h"
#include <iostream>

Benchmark::Benchmark(const int runtimeMs, const int allocation_size, const int alignment_size){
    m_runtimeMs = runtimeMs;
    m_allocationSize = allocation_size;
    m_alignmentSize = alignment_size;
}

BenchmarkResults Benchmark::Allocation(Allocator* allocator){
    std::cout << "BENCHMARK: ALLOCATION" << std::endl;
    void setStartTimer();

    int operations = 0;
    while(!outOfTime()){
        //allocator->Allocate(m_allocationSize, m_alignmentSize);
        ++operations;
    }
    BenchmarkResults results = buildResults(operations, m_runtimeMs, 0,0);
    printResults(results);
    return results;
}


void Benchmark::setStartTimer(){
	setTimer(m_start);
}

void Benchmark::setTimer(timespec& timer){
    clock_gettime(CLOCK_REALTIME, &timer);
}

const bool Benchmark::outOfTime() {
    timespec now;
    setTimer(now);

    double elapsedTime = calculateElapsedTime(m_start, now);
    if (elapsedTime > m_runtimeMs){
        return true;
    }
    std::cout << elapsedTime << std::endl;
    return false;
}

const double Benchmark::calculateElapsedTime(const timespec& start, const timespec& end) const {
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1e9+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }

    const double time_sec = (double) temp.tv_sec;
    const double time_nsec = (double) temp.tv_nsec;
    const double time_msec = (time_sec * 1e3) + (time_nsec / 1e6);

    return time_msec;
}

void Benchmark::printResults(const BenchmarkResults& results) const {
	std::cout << "\tRESULTS:" << std::endl;
	std::cout << "\t\tOperations:    \t" << results.nOperations  << std::endl;
	std::cout << "\t\tTime elapsed:  \t" << results.elapsedTime << " ms" << std::endl;
	std::cout << "\t\tOp per sec:    \t" << results.operationsPerSec << " ops/ms" << std::endl;
	std::cout << "\t\tTimer per op:  \t" << results.timePerOperation << " ms/ops" << std::endl;
	if (results.memoryUsed > 0) {
        std::cout << "\t\tMemory used:   \t" << results.memoryUsed  << " bytes" << std::endl;
        std::cout << "\t\tMemory wasted: \t" << results.memoryWasted  << " bytes\t" << "\t" << ((float) results.memoryWasted / results.memoryUsed)*100 << "%" << std::endl;
	}
  std::cout << std::endl;
}

const BenchmarkResults Benchmark::buildResults(const long nOperations, const double elapsedTime, const std::size_t memoryUsed, const std::size_t memoryWasted) const{
	BenchmarkResults results;

	results.nOperations = nOperations;
	results.elapsedTime = elapsedTime;
	results.operationsPerSec = results.nOperations / results.elapsedTime;
	results.timePerOperation = results.elapsedTime / results.nOperations;
	results.memoryUsed = memoryUsed;
	results.memoryWasted = memoryWasted;

	return results;
}


