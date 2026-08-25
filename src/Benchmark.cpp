#include "Benchmark.h"
#include <iostream>
#include <cassert>

Benchmark::Benchmark(const unsigned int nOperations) noexcept : m_nOperations(nOperations), m_rng(42), m_elapsed(0) {}

void Benchmark::SingleAllocation(Allocator* allocator, const std::size_t size, const std::size_t alignment) {
    std::cout << "BENCHMARK: ALLOCATION" << IO::endl;
    std::cout << "\tSize:      \t" << size << IO::endl;
    std::cout << "\tAlignment: \t" << alignment << IO::endl;

    allocator->Init();
    StartRound();

    for (unsigned int i = 0; i < m_nOperations; ++i)
        allocator->Allocate(size, alignment);

    FinishRound();
    PrintResults(BuildResults(m_nOperations, m_elapsed, allocator->m_peak));
}

void Benchmark::SingleFree(Allocator* allocator, const std::size_t size, const std::size_t alignment) {
    std::cout << "BENCHMARK: ALLOCATION/FREE" << IO::endl;
    std::cout << "\tSize:      \t" << size << IO::endl;
    std::cout << "\tAlignment: \t" << alignment << IO::endl;

    std::vector<void*> addresses(m_nOperations);

    allocator->Init();
    StartRound();

    for (unsigned int i = 0; i < m_nOperations; ++i)
        addresses[i] = allocator->Allocate(size, alignment);

    for (unsigned int i = m_nOperations; i > 0; --i)
        allocator->Free(addresses[i - 1]);

    FinishRound();
    PrintResults(BuildResults(m_nOperations, m_elapsed, allocator->m_peak));
}

void Benchmark::MultipleAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments) {
    assert(allocationSizes.size() == alignments.size() && "Allocation sizes and alignments must have the same length");

    for (std::size_t i = 0; i < allocationSizes.size(); ++i) {
        SingleAllocation(allocator, allocationSizes[i], alignments[i]);
    }
}

void Benchmark::MultipleFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments) {
    assert(allocationSizes.size() == alignments.size() && "Allocation sizes and alignments must have the same length");

    for (std::size_t i = 0; i < allocationSizes.size(); ++i) {
        SingleFree(allocator, allocationSizes[i], alignments[i]);
    }
}

void Benchmark::RandomAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments) {
    std::cout << "\tBENCHMARK: RANDOM ALLOCATION" << IO::endl;

    allocator->Init();
    StartRound();

    std::size_t allocation_size;
    std::size_t alignment;

    for (unsigned int i = 0; i < m_nOperations; ++i) {
        RandomAllocationAttr(allocationSizes, alignments, allocation_size, alignment);

        allocator->Allocate(allocation_size, alignment);
    }

    FinishRound();
    PrintResults(BuildResults(m_nOperations, m_elapsed, allocator->m_peak));
}

void Benchmark::RandomFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments) {
    std::cout << "\tBENCHMARK: RANDOM ALLOCATION/FREE" << IO::endl;

    std::vector<void*> addresses(m_nOperations);
    std::size_t allocation_size;
    std::size_t alignment;

    allocator->Init();
    StartRound();

    for (unsigned int i = 0; i < m_nOperations; ++i) {
        RandomAllocationAttr(allocationSizes, alignments, allocation_size, alignment);

        addresses[i] = allocator->Allocate(allocation_size, alignment);
    }

    for (unsigned int i = m_nOperations; i > 0; --i)
        allocator->Free(addresses[i - 1]);

    FinishRound();
    PrintResults(BuildResults(m_nOperations, m_elapsed, allocator->m_peak));
}

void Benchmark::PrintResults(const BenchmarkResults& results) const {
    const double ns = static_cast<double>(results.Nanoseconds.count());

    std::cout << "\tRESULTS:" << IO::endl;
    std::cout << "\t\tOperations:    \t" << results.Operations << IO::endl;
    std::cout << "\t\tTime elapsed: \t" << ns / 1e6 << " ms" << IO::endl;
    std::cout << "\t\tOp per sec:    \t" << results.OperationsPerSec << " ops/ns" << IO::endl;
    std::cout << "\t\tTime per op:   \t" << results.TimePerOperation << " ns/op" << IO::endl;
    std::cout << "\t\tMemory peak:   \t" << results.MemoryPeak << " bytes" << IO::endl;

    std::cout << IO::endl;
}

BenchmarkResults Benchmark::BuildResults(const std::size_t nOperations, const std::chrono::nanoseconds elapsedTime, const std::size_t memoryPeak) const noexcept {
    BenchmarkResults results;

    results.Operations = nOperations;
    results.Nanoseconds = elapsedTime;

    const double ns = static_cast<double>(elapsedTime.count());

    results.OperationsPerSec = ns > 0.0 ? static_cast<double>(nOperations) / ns : 0.0;
    results.TimePerOperation = ns > 0.0 ? ns / static_cast<double>(nOperations) : 0.0;
    results.MemoryPeak = memoryPeak;

    return results;
}

void Benchmark::RandomAllocationAttr(const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments, std::size_t& size, std::size_t& alignment) noexcept {
    std::uniform_int_distribution<std::size_t> dist(0, allocationSizes.size() - 1);

    const std::size_t idx = dist(m_rng);

    size = allocationSizes[idx];
    alignment = alignments[idx];
}