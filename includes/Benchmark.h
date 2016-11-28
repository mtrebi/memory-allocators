#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <time.h> // timespec
#include <cstddef> // std::size_t
#include <vector>
#include "Allocator.h" // base class allocator

struct foo {
    char *p;     /* 8 bytes */
    char c;      /* 1 byte */
};

struct bar {
	int a;		// 4
	bool b;		// 1 -> 5
				// 3 -> 8
	int c;		// 4 -> 12
	bool d;		// 1 -> 13
	bool e;		// 1 -> 14
				// 2 -> 16
};

struct bar2 {
	int a;	// 4
	int c;	// 4 -> 8
	bool b;	// 1 -> 9
	bool d;
	bool e;
			// 3 -> 12
};

struct foo3 {
	int i;      /* 4 byte */
    char c;     /* 1 bytes */
    bool b;		/* 1 bytes */
    			// 2 bytes
};

struct foo2 {
	char c;      /* 1 byte */
    char *p;     /* 8 bytes */
};

struct baz {
    short s;     /* 2 bytes */
    char c;      /* 1 byte */
};

struct BenchmarkResults {
	long nOperations;
	double elapsedTime;
	float operationsPerSec;
	float timePerOperation;
	int memoryUsed;
};

class Benchmark {
public:
	Benchmark(const unsigned int nOperations, const std::vector<std::size_t>& allocationSizes, const std::size_t alignment);

	void Allocation(Allocator* allocator);
	void Free(Allocator* allocator);
private:
	void printResults(const BenchmarkResults& results) const;
	void setTimer(timespec& timer);

	const double calculateElapsedTime() const;
	const BenchmarkResults buildResults(const unsigned int nOperations, const double elapsedTime, const std::size_t memoryUsed) const;
private:
	unsigned int m_nOperations,
		m_alignment;
	std::vector<std::size_t> m_allocationSizes;
	timespec m_start, m_end;
};

#endif /* BENCHMARK_H */