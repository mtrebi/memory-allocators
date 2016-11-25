#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <time.h> // timespec

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

struct Benchmark_results {
	long nOperations;
	timespec elapsedTime;
	float operationsPerSec;
	float timePerOperation;
	int memoryWasted;
	int memoryUsed;
};

class Benchmark {
private:
	long m_nOperations;
	timespec m_start;
	timespec m_end;
public:
	Benchmark(long nOperations);

	virtual Benchmark_results allocation() = 0;
	virtual Benchmark_results free() = 0;
	virtual Benchmark_results read() = 0;
	virtual Benchmark_results write() = 0;
	virtual Benchmark_results all() = 0;
protected:
	void printResults(const Benchmark_results& results) const;
	void setStartTimer();
	void setEndTimer();
	const timespec calculateElapsedTime() const;
	const Benchmark_results buildResults(const long nOperations, const timespec elapsedTime, const std::size_t memoryUsed, const std::size_t memoryWasted) const;
private:
	void setTimer(timespec& timer);

};

#endif /* BENCHMARK_H */