#include "Benchmark.h"
#include <iostream>
#include <cstdlib>      /* std::rand, std::srand */
#include <cassert>
#include <vector>

void Benchmark::SingleAllocation(Allocator* allocator, const std::size_t size, const std::size_t alignment) {
    assert(allocator != nullptr && "allocator must not be null");

    std::cout << "BENCHMARK: ALLOCATION" << IO::endl;
    std::cout << "\tSize:     \t" << size << IO::endl;
    std::cout << "\tAlignment\t" << alignment << IO::endl;

    StartRound();

    allocator->Init();

    std::size_t operations = 0;

    while (operations < m_nOperations) {
        // D-28: 原实现丢弃返回值，分配失败与成功在统计上无法区分
        (void) allocator->Allocate(size, alignment);
        ++operations;
    }
    
    FinishRound();

    BenchmarkResults results = buildResults(m_nOperations, std::move(TimeElapsed), allocator->m_peak);

    PrintResults(results);
}

void Benchmark::SingleFree(Allocator* allocator, const std::size_t size, const std::size_t alignment) {
    assert(allocator != nullptr && "allocator must not be null");

    std::cout << "BENCHMARK: ALLOCATION/FREE" << IO::endl;
    std::cout << "\tSize:     \t" << size << IO::endl;
    std::cout << "\tAlignment\t" << alignment << IO::endl;

    // D-14: 原实现是 "void* addresses[OPERATIONS];" 且 OPERATIONS 被硬编码为 10。
    // m_nOperations 由构造函数决定，一旦大于 10 就是栈缓冲区溢出。
    std::vector<void*> addresses(m_nOperations, nullptr);

    StartRound();

    allocator->Init();

    std::size_t operations = 0;

    while (operations < m_nOperations) {
        addresses[operations] = allocator->Allocate(size, alignment);
        ++operations;
    }

    if (allocator->IsFreeable()) {   // D-31: 线性分配器不支持单次释放
        while (operations) {
            allocator->Free(addresses[--operations]);
        }
    }

    FinishRound();

    BenchmarkResults results = buildResults(m_nOperations, std::move(TimeElapsed), allocator->m_peak);

    PrintResults(results);
}

void Benchmark::MultipleAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments) {
    assert(allocationSizes.size() == alignments.size() && "Allocation sizes and Alignments must have same length");

    for (std::size_t i = 0; i < allocationSizes.size(); ++i) {
        SingleAllocation(allocator, allocationSizes[i], alignments[i]);
    }
}

void Benchmark::MultipleFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments) {
    assert(allocationSizes.size() == alignments.size() && "Allocation sizes and Alignments must have same length");

    for (std::size_t i = 0; i < allocationSizes.size(); ++i) {
        SingleFree(allocator, allocationSizes[i], alignments[i]);
    }
}

void Benchmark::RandomAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments) {
    assert(allocator != nullptr && "allocator must not be null");
    assert(allocationSizes.size() == alignments.size() && "Allocation sizes and Alignments must have same length");

    std::srand(1);

    std::cout << "\tBENCHMARK: ALLOCATION" << IO::endl;

    StartRound();

    std::size_t allocation_size = 0;
    std::size_t alignment = 0;

    allocator->Init();

    std::size_t operations = 0;

    while (operations < m_nOperations) {
        this->RandomAllocationAttr(allocationSizes, alignments, allocation_size, alignment);
        (void) allocator->Allocate(allocation_size, alignment);
        ++operations;
    }
    
    FinishRound();

    BenchmarkResults results = buildResults(m_nOperations, std::move(TimeElapsed), allocator->m_peak);
    
    PrintResults(results);
}

void Benchmark::RandomFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments) {
    assert(allocator != nullptr && "allocator must not be null");
    assert(allocationSizes.size() == alignments.size() && "Allocation sizes and Alignments must have same length");

    std::srand(1);

    std::cout << "\tBENCHMARK: ALLOCATION/FREE" << IO::endl;

    StartRound();

    // D-14: 同上，改用按 m_nOperations 定长的容器
    std::vector<void*> addresses(m_nOperations, nullptr);

    std::size_t allocation_size = 0;
    std::size_t alignment = 0;

    allocator->Init();

    std::size_t operations = 0;

    while (operations < m_nOperations) {
        this->RandomAllocationAttr(allocationSizes, alignments, allocation_size, alignment);
        addresses[operations] = allocator->Allocate(allocation_size, alignment);
        ++operations;
    }

    if (allocator->IsFreeable()) {   // D-31
        while (operations) {
            allocator->Free(addresses[--operations]);
        }
    }

    FinishRound();

    BenchmarkResults results = buildResults(m_nOperations, std::move(TimeElapsed), allocator->m_peak);

    PrintResults(results);

}

void Benchmark::PrintResults(const BenchmarkResults& results) const {
    std::cout << "\tRESULTS:" << IO::endl;
    std::cout << "\t\tOperations:    \t" << results.Operations << IO::endl;
    std::cout << "\t\tTime elapsed:  \t" << results.Nanoseconds.count() << " ns" << IO::endl;
    std::cout << "\t\tOp per sec:    \t" << results.OperationsPerSec << " ops/s" << IO::endl;
    std::cout << "\t\tTime per op:   \t" << results.NanosecondsPerOperation << " ns/op" << IO::endl;
    std::cout << "\t\tMemory peak:   \t" << results.MemoryPeak << " bytes" << IO::endl;

    std::cout << IO::endl;
}

const BenchmarkResults Benchmark::buildResults(std::size_t nOperations, std::chrono::nanoseconds&& elapsedTime, const std::size_t memoryPeak) const {
    BenchmarkResults results;

    results.Operations = nOperations;
    results.Nanoseconds = std::move(elapsedTime);

    // D-13: 原实现直接做 "Operations / Milliseconds.count()"，
    // 计时精度不足时除数为 0，结果是 inf / nan。
    const double ns = static_cast<double>(results.Nanoseconds.count());
    if (ns > 0.0) {
        results.OperationsPerSec = static_cast<double>(nOperations) * 1e9 / ns;
        results.NanosecondsPerOperation = ns / static_cast<double>(nOperations);
    } else {
        results.OperationsPerSec = 0.0;
        results.NanosecondsPerOperation = 0.0;
    }
    results.MemoryPeak = memoryPeak;

    return results;
}

void Benchmark::RandomAllocationAttr(const std::vector<std::size_t>& allocationSizes, const std::vector<std::size_t>& alignments, std::size_t & size, std::size_t & alignment) {
    // D-21: 原实现 "rand() % allocationSizes.size()" 是有符号/无符号混合运算，
    // 且未防护空容器（模 0 未定义）。改用无符号索引并加断言。
    assert(!allocationSizes.empty() && "allocationSizes must not be empty");
    const std::size_t r = static_cast<std::size_t>(std::rand()) % allocationSizes.size();
    size = allocationSizes[r];
    alignment = alignments[r];
}
