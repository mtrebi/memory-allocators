#include "Benchmark.h"
#include <iostream>

Benchmark::Benchmark(const unsigned int nOperations, const std::vector<std::size_t>& allocationSizes, const std::size_t alignment){
    m_nOperations = nOperations;
    m_alignment = alignment;
    m_allocationSizes = allocationSizes;
}


void Benchmark::Allocation(Allocator* allocator){
    for (auto& allocation_size : m_allocationSizes) {
        std::cout << "\tBENCHMARK: ALLOCATION" <<  std::endl;
        std::cout << "\tSize:     \t" << allocation_size << std::endl;
        std::cout << "\tAlignment\t" << m_alignment <<  std::endl;

        setTimer(m_start);
        allocator->Init();
        unsigned int operations = 0;
        while(operations < m_nOperations){
            allocator->Allocate(allocation_size, m_alignment);
            ++operations;
        }
        setTimer(m_end);

        BenchmarkResults results = buildResults(m_nOperations, calculateElapsedTime(), allocator->m_used);
        printResults(results);
    }
}

void Benchmark::Free(Allocator* allocator){
    for (auto& allocation_size : m_allocationSizes) {
        std::cout << "BENCHMARK: ALLOCATION/FREE" <<  std::endl;
        std::cout << "\tSize:     \t" << allocation_size << std::endl;
        std::cout << "\tAlignment\t" << m_alignment <<  std::endl;

        void* addresses[m_nOperations];

        setTimer(m_start);
        allocator->Init();
        int operations = 0;
        while(operations < m_nOperations){
            addresses[operations] = allocator->Allocate(allocation_size, m_alignment);
            ++operations;
        }
        --operations;
        while(operations >= 0){
            allocator->Free(addresses[operations]);
            --operations;
        }

        setTimer(m_end);

        BenchmarkResults results = buildResults(m_nOperations, calculateElapsedTime(), allocator->m_used);
        printResults(results);
    }
}

void Benchmark::setTimer(timespec& timer){
    clock_gettime(CLOCK_REALTIME, &timer);
}

const double Benchmark::calculateElapsedTime() const {
    timespec temp;
    if ((m_end.tv_nsec-m_start.tv_nsec) < 0) {
        temp.tv_sec = m_end.tv_sec-m_start.tv_sec-1;
        temp.tv_nsec = 1e9+m_end.tv_nsec-m_start.tv_nsec;
    } else {
        temp.tv_sec = m_end.tv_sec-m_start.tv_sec;
        temp.tv_nsec = m_end.tv_nsec-m_start.tv_nsec;
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
    std::cout << "\t\tMemory used:   \t" << results.memoryUsed  << " bytes" << std::endl;
	
  std::cout << std::endl;
}

const BenchmarkResults Benchmark::buildResults(const unsigned int nOperations, const double elapsedTime, const std::size_t memoryUsed) const{
	BenchmarkResults results;

	results.nOperations = nOperations;
	results.elapsedTime = elapsedTime;
	results.operationsPerSec = results.nOperations / results.elapsedTime;
	results.timePerOperation = results.elapsedTime / results.nOperations;
	results.memoryUsed = memoryUsed;

	return results;
}


