#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <chrono>
#include <cstddef> // std::size_t
#include <random>
#include <vector>

#include "Allocator.h" // base class allocator
#include "IO.h"

struct BenchmarkResults
{
    std::size_t Operations;
    std::chrono::nanoseconds Nanoseconds;
    double OperationsPerSec;
    double TimePerOperation;
    std::size_t MemoryPeak;
};

class Benchmark {
public:
    Benchmark() = delete;
    explicit Benchmark(unsigned int nOperations) noexcept;

    void SingleAllocation(Allocator* allocator, std::size_t size, std::size_t alignment);
    void SingleFree(Allocator* allocator, std::size_t size, std::size_t alignment);

    void MultipleAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);
    void MultipleFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);

    void RandomAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);
    void RandomFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments);

private:
    void PrintResults(const BenchmarkResults& results) const;

    void RandomAllocationAttr(const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments, std::size_t& size, std::size_t& alignment) noexcept;

    BenchmarkResults BuildResults(std::size_t nOperations, std::chrono::nanoseconds elapsedTime, std::size_t memoryPeak) const noexcept;

    void StartRound() noexcept  {
        m_start = std::chrono::high_resolution_clock::now();
    }

    void FinishRound() noexcept {
        m_finish = std::chrono::high_resolution_clock::now();
        m_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(m_finish - m_start);
    }

    unsigned int m_nOperations;
    std::mt19937 m_rng;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_finish;
    std::chrono::nanoseconds m_elapsed;
};

#endif /* BENCHMARK_H */