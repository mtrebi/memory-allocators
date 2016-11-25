#ifndef BENCHMARKSTACK_H
#define BENCHMARKSTACK_H

#include "Benchmark.h"

class BenchmarkStack : public Benchmark {
public:
	BenchmarkStack(const int runtime);

	virtual BenchmarkResults allocation() override;
	virtual BenchmarkResults freeing() override;
	virtual BenchmarkResults read() override;
	virtual BenchmarkResults write() override;
	virtual BenchmarkResults all() override;
};

#endif /* BENCHMARKSTACK_H */