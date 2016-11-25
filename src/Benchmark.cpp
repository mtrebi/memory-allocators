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

void Benchmark::print_results(const Benchmark_results& results) const {
	double time_sec = (double) results.elapsedTime.tv_sec;
  	double time_nsec = (double) results.elapsedTime.tv_nsec;
  	double time_msec = (time_sec * 1000) + (time_nsec / 1000000);

	std::cout << std::endl;
	std::cout << "\tRESULTS:" << std::endl;
	std::cout << "\t\tOperations:    \t" << results.nOperations  << std::endl;
	std::cout << "\t\tTime elapsed:  \t" << time_msec << " ms" << std::endl;
	std::cout << "\t\tOp per sec:    \t" << (results.nOperations / 1e3) / time_msec << " mops/ms" << std::endl;
	std::cout << "\t\tTimer per op:  \t" << time_msec/(results.nOperations / 1e3) << " ms/mops" << std::endl;
	std::cout << "\t\tMemory used:   \t" << results.memoryWasted  << " bytes" << std::endl;
	std::cout << "\t\tMemory wasted: \t" << results.memoryUsed  << " bytes\t" << ((float) results.memoryWasted / results.memoryUsed) * 100 << " %" << std::endl;
	std::cout << std::endl;
}