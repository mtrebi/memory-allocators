#include "Benchmark.h"
#include <iostream>

Benchmark::Benchmark(const int runtime){
  m_runtime = runtime;
}

void Benchmark::setStartTimer(){
	setTimer(m_start);
}

void Benchmark::setTimer(timespec& timer){
  clock_gettime(CLOCK_REALTIME, &timer);
}

const double Benchmark::calculateElapsedTime(const timespec& start, const timespec& end) const {
  timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
      temp.tv_sec = end.tv_sec-start.tv_sec-1;
      temp.tv_nsec = 1e9+end.tv_nsec-start.tv_nsec;
  } else {
      temp.tv_sec = end.tv_sec-start.tv_sec;
      temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }

  const double time_sec = (double) temp.tv_sec;
  const double time_nsec = (double) temp.tv_nsec;
  const double time_msec = (time_sec * 1e3) + (time_nsec / 1e6);

  return time_msec;
}

const bool Benchmark::outOfTime() {
  timespec now;
  setTimer(now);

  double elapsedTime = calculateElapsedTime(m_start, now);
  if (elapsedTime > (m_runtime * 1e3)){
    return true;
  }
  return false;
}


void Benchmark::printResults(const BenchmarkResults& results) const {
	std::cout << "\tRESULTS:" << std::endl;
	std::cout << "\t\tOperations:    \t" << results.nOperations  << std::endl;
	std::cout << "\t\tTime elapsed:  \t" << results.elapsedTime << " s" << std::endl;
	std::cout << "\t\tOp per sec:    \t" << results.operationsPerSec << " ops/s" << std::endl;
	std::cout << "\t\tTimer per op:  \t" << results.timePerOperation << " s/ops" << std::endl;
	std::cout << "\t\tMemory used:   \t" << results.memoryWasted  << " bytes" << std::endl;
	std::cout << "\t\tMemory wasted: \t" << results.memoryUsed  << " bytes\t" << std::endl;
	std::cout << std::endl;
}

const BenchmarkResults Benchmark::buildResults(const long nOperations, const double elapsedTime, const std::size_t memoryUsed, const std::size_t memoryWasted) const{
	BenchmarkResults results;

	results.nOperations = nOperations;
	results.elapsedTime = elapsedTime;
	results.operationsPerSec = results.nOperations / results.elapsedTime;
	results.timePerOperation = results.elapsedTime / results.nOperations;
	results.memoryUsed = memoryUsed;
	results.memoryWasted = memoryWasted;

	return results;
}