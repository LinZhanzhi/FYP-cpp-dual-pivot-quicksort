CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -DNDEBUG -Ibenchmarks/include -Iinclude
BUILD_DIR = benchmarks/build
SRC_DIR = benchmarks/src
RUNNER = benchmarks/build/benchmark_runner

all: runner

runner:
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(RUNNER) $(SRC_DIR)/benchmark_runner.cpp -pthread

run: runner
	cd benchmarks && python3 benchmark_manager.py

clean:
	rm -rf $(BUILD_DIR)
	rm -rf benchmarks/results/raw/*

.PHONY: all runner run clean
