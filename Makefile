# Makefile for Dual-Pivot Quicksort Benchmark

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include
OPTIMIZE_FLAGS = -O3 -march=native -DNDEBUG
DEBUG_FLAGS = -g -O0 -DDEBUG
BENCHMARK_DIR = benchmarks
RESULTS_DIR = results

# Default target
all: benchmark_optimized

# Create directories
$(RESULTS_DIR):
	mkdir -p $(RESULTS_DIR)

# Optimized benchmark (for performance testing)
benchmark_optimized: $(BENCHMARK_DIR)/main_benchmark.cpp $(RESULTS_DIR)
	$(CXX) $(CXXFLAGS) $(OPTIMIZE_FLAGS) -o $(RESULTS_DIR)/benchmark_optimized $(BENCHMARK_DIR)/main_benchmark.cpp

# Debug benchmark (for development)
benchmark_debug: $(BENCHMARK_DIR)/main_benchmark.cpp $(RESULTS_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $(RESULTS_DIR)/benchmark_debug $(BENCHMARK_DIR)/main_benchmark.cpp

# Run benchmark with different optimization levels
run_benchmark: benchmark_optimized
	@echo "Running benchmark with -O3 optimization..."
	cd $(RESULTS_DIR) && ./benchmark_optimized
	@echo "Benchmark completed. Results in $(RESULTS_DIR)/benchmark_results.csv"

# Run multiple optimization levels
run_all_optimizations: 
	@echo "Building and running with -O0..."
	$(CXX) $(CXXFLAGS) -O0 -o $(RESULTS_DIR)/benchmark_O0 $(BENCHMARK_DIR)/main_benchmark.cpp
	cd $(RESULTS_DIR) && ./benchmark_O0 && mv benchmark_results.csv benchmark_results_O0.csv
	
	@echo "Building and running with -O2..."
	$(CXX) $(CXXFLAGS) -O2 -o $(RESULTS_DIR)/benchmark_O2 $(BENCHMARK_DIR)/main_benchmark.cpp
	cd $(RESULTS_DIR) && ./benchmark_O2 && mv benchmark_results.csv benchmark_results_O2.csv
	
	@echo "Building and running with -O3..."
	$(CXX) $(CXXFLAGS) -O3 -march=native -o $(RESULTS_DIR)/benchmark_O3 $(BENCHMARK_DIR)/main_benchmark.cpp
	cd $(RESULTS_DIR) && ./benchmark_O3 && mv benchmark_results.csv benchmark_results_O3.csv

# Simple unit tests
test: $(RESULTS_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -DUNIT_TEST -o $(RESULTS_DIR)/unit_test $(BENCHMARK_DIR)/main_benchmark.cpp
	cd $(RESULTS_DIR) && ./unit_test

# Clean build artifacts
clean:
	rm -rf $(RESULTS_DIR)/*

# Help target
help:
	@echo "Available targets:"
	@echo "  all                    - Build optimized benchmark (default)"
	@echo "  benchmark_optimized    - Build with -O3 optimization"
	@echo "  benchmark_debug        - Build with debug symbols"
	@echo "  run_benchmark         - Run the benchmark suite"
	@echo "  run_all_optimizations - Run benchmarks with different optimization levels"
	@echo "  test                  - Run unit tests"
	@echo "  clean                 - Clean build artifacts"
	@echo "  help                  - Show this help message"

.PHONY: all benchmark_optimized benchmark_debug run_benchmark run_all_optimizations test clean help