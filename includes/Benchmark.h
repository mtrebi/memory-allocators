#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <cstddef> // std::size_t
#include <chrono>
#include <vector>

#include "Allocator.h" // base class allocator
#include "IO.h"

struct BenchmarkResults
{
	std::size_t Operations;
	// D-12: 原实现用 std::chrono::milliseconds 计时。单次分配只有几十纳秒，
	// 10 次操作合计不足 1 ms，duration_cast 后恒为 0，随后 "Operations / 0"
	// 得到 inf，基准数据完全失去意义。改用纳秒。
	std::chrono::nanoseconds Nanoseconds;
	double OperationsPerSec;
	double NanosecondsPerOperation;
	std::size_t MemoryPeak;
};

class Benchmark {
public:
    Benchmark() = delete;

    Benchmark(const unsigned int nOperations) : m_nOperations { nOperations } { }

	void SingleAllocation(Allocator* allocator, const std::size_t size, const std::size_t alignment);
	void SingleFree(Allocator* allocator, const std::size_t size, const std::size_t alignment);

	void MultipleAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);
	void MultipleFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);

	void RandomAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);
	void RandomFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);

private:
	void PrintResults(const BenchmarkResults& results) const;

	void RandomAllocationAttr(const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments, std::size_t & size, std::size_t & alignment);

	const BenchmarkResults buildResults(std::size_t nOperations, std::chrono::nanoseconds&& elapsedTime, const std::size_t memoryPeak) const;
    
    void SetStartTime() noexcept { Start = std::chrono::high_resolution_clock::now(); }

    void SetFinishTime() noexcept { Finish = std::chrono::high_resolution_clock::now(); }

    void SetElapsedTime() noexcept { TimeElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(Finish - Start); }

    void StartRound() noexcept { SetStartTime(); }

    void FinishRound() noexcept
    {
        SetFinishTime();
        SetElapsedTime();
    }

private:
	std::size_t m_nOperations;

    std::chrono::time_point<std::chrono::high_resolution_clock> Start;
    std::chrono::time_point<std::chrono::high_resolution_clock> Finish;

    std::chrono::nanoseconds TimeElapsed;
};

#endif /* BENCHMARK_H */
