#ifndef BENCHMARKC_H
#define BENCHMARKC_H

#include "Benchmark.h"

class BenchmarkC : public Benchmark {
public:
	BenchmarkC(const int runtime);

	virtual BenchmarkResults allocation() override;
	virtual BenchmarkResults free() override;
	virtual BenchmarkResults read() override;
	virtual BenchmarkResults write() override;
	virtual BenchmarkResults all() override;
};

#endif /* BENCHMARKC_H */