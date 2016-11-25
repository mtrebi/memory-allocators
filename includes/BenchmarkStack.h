#ifndef BENCHMARKSTACK_H
#define BENCHMARKSTACK_H

#include "Benchmark.h"

class BenchmarkStack : public Benchmark {
public:
	BenchmarkStack(long nOperations);

	virtual BenchmarkResults allocation() override;
	virtual BenchmarkResults free() override;
	virtual BenchmarkResults read() override;
	virtual BenchmarkResults write() override;
	virtual BenchmarkResults all() override;
};

#endif /* BENCHMARKSTACK_H */