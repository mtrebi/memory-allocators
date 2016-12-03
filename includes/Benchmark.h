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
        int memoryPeak;
};

class Benchmark {
public:
	Benchmark(const unsigned int nOperations);

	void SingleAllocation(Allocator* allocator, const std::size_t size, const std::size_t alignment);
	void SingleFree(Allocator* allocator, const std::size_t size, const std::size_t alignment);

	void MultipleAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);
	void MultipleFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);

	void RandomAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);
	void RandomFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);
private:
	void printResults(const BenchmarkResults& results) const;
	void setTimer(timespec& timer);
	void RandomAllocationAttr(const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments, std::size_t & size, std::size_t & alignment);

	const double calculateElapsedTime() const;
	const BenchmarkResults buildResults(const unsigned int nOperations, const double elapsedTime, const std::size_t memoryUsed) const;
private:
	unsigned int m_nOperations;
	timespec m_start, m_end;
};

#endif /* BENCHMARK_H */