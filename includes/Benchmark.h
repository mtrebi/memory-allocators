#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <time.h> // timespec

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