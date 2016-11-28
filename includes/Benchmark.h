#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <time.h> // timespec
#include <cstddef> // std::size_t
#include <vector>
#include "Allocator.h" // base class allocator

struct BenchmarkResults {
	long nOperations;
	double elapsedTime;
	float operationsPerSec;
	float timePerOperation;
	int memoryUsed;
};

class Benchmark {
public:
	Benchmark(const unsigned int nOperations, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);

	void Allocation(Allocator* allocator);
	void Free(Allocator* allocator);

	void AllocationRandom(Allocator* allocator);
	void FreeRandom(Allocator* allocator);
private:
	void printResults(const BenchmarkResults& results) const;
	void setTimer(timespec& timer);
	void RandomAllocationAttr(std::size_t & size, std::size_t & alignment);

	const double calculateElapsedTime() const;
	const BenchmarkResults buildResults(const unsigned int nOperations, const double elapsedTime, const std::size_t memoryUsed) const;
private:
	unsigned int m_nOperations;
	std::vector<std::size_t> m_allocationSizes;
	std::vector<std::size_t> m_alignments;

	timespec m_start, m_end;
};

#endif /* BENCHMARK_H */