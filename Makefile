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
	$(CXX) $(CXXFLAGS) $(OPTIMIZE_FLAGS) -o $(RESULTS_DIR)/benchmark_optimized.exe $(BENCHMARK_DIR)/main_benchmark.cpp

# Debug benchmark (for development)
benchmark_debug: $(BENCHMARK_DIR)/main_benchmark.cpp $(RESULTS_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $(RESULTS_DIR)/benchmark_debug.exe $(BENCHMARK_DIR)/main_benchmark.cpp

# Quick benchmark (for demonstration)
quick_benchmark: $(BENCHMARK_DIR)/quick_benchmark.cpp $(RESULTS_DIR)
	$(CXX) $(CXXFLAGS) $(OPTIMIZE_FLAGS) -o $(RESULTS_DIR)/quick_benchmark.exe $(BENCHMARK_DIR)/quick_benchmark.cpp

# Multi-type benchmark (for data type analysis)
multi_type_benchmark: $(BENCHMARK_DIR)/multi_type_benchmark.cpp $(RESULTS_DIR)
	$(CXX) $(CXXFLAGS) $(OPTIMIZE_FLAGS) -o $(RESULTS_DIR)/multi_type_benchmark.exe $(BENCHMARK_DIR)/multi_type_benchmark.cpp

# Multi-type benchmark debug version
multi_type_benchmark_debug: $(BENCHMARK_DIR)/multi_type_benchmark.cpp $(RESULTS_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -o $(RESULTS_DIR)/multi_type_benchmark_debug.exe $(BENCHMARK_DIR)/multi_type_benchmark.cpp

# Run quick benchmark with plotting
run_quick_benchmark: quick_benchmark
	@echo "Running quick benchmark demonstration..."
	cd $(RESULTS_DIR) && quick_benchmark.exe
	@echo "Quick benchmark completed with plots!"

# Run multi-type benchmark
run_multi_type_benchmark: multi_type_benchmark
	@echo "Running multi-type benchmark analysis..."
	cd $(RESULTS_DIR) && multi_type_benchmark.exe
	@echo "Multi-type benchmark completed. Results in $(RESULTS_DIR)/multi_type_benchmark_results.csv"
	@echo "Performance plots saved in $(RESULTS_DIR)/plots/"

# Run benchmark with different optimization levels
run_benchmark: benchmark_optimized
	@echo "Running benchmark with -O3 optimization..."
	cd $(RESULTS_DIR) && benchmark_optimized.exe
	@echo "Benchmark completed. Results in $(RESULTS_DIR)/benchmark_results.csv"
	@echo "Performance plots saved in $(RESULTS_DIR)/plots/"

# Run multiple optimization levels
run_all_optimizations: 
	@echo "Building and running with -O0..."
	$(CXX) $(CXXFLAGS) -O0 -o $(RESULTS_DIR)/benchmark_O0.exe $(BENCHMARK_DIR)/main_benchmark.cpp
	cd $(RESULTS_DIR) && benchmark_O0.exe && mv benchmark_results.csv benchmark_results_O0.csv
	
	@echo "Building and running with -O2..."
	$(CXX) $(CXXFLAGS) -O2 -o $(RESULTS_DIR)/benchmark_O2.exe $(BENCHMARK_DIR)/main_benchmark.cpp
	cd $(RESULTS_DIR) && benchmark_O2.exe && mv benchmark_results.csv benchmark_results_O2.csv
	
	@echo "Building and running with -O3..."
	$(CXX) $(CXXFLAGS) -O3 -march=native -o $(RESULTS_DIR)/benchmark_O3.exe $(BENCHMARK_DIR)/main_benchmark.cpp
	cd $(RESULTS_DIR) && benchmark_O3.exe && mv benchmark_results.csv benchmark_results_O3.csv

# Simple unit tests
test: $(RESULTS_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -DUNIT_TEST -o $(RESULTS_DIR)/unit_test.exe $(BENCHMARK_DIR)/main_benchmark.cpp
	cd $(RESULTS_DIR) && unit_test.exe

# Clean build artifacts
clean:
	rm -rf $(RESULTS_DIR)/*

# Help target
help:
	@echo "Available targets:"
	@echo "  all                       - Build optimized benchmark (default)"
	@echo "  benchmark_optimized       - Build with -O3 optimization"
	@echo "  benchmark_debug           - Build with debug symbols"
	@echo "  quick_benchmark           - Build quick demo benchmark"
	@echo "  multi_type_benchmark      - Build multi-type analysis benchmark"
	@echo "  multi_type_benchmark_debug - Build multi-type benchmark with debug"
	@echo "  run_quick_benchmark       - Run quick benchmark with plots (recommended)"
	@echo "  run_multi_type_benchmark  - Run multi-type analysis and generate plots"
	@echo "  run_benchmark             - Run full benchmark suite and generate plots"
	@echo "  run_all_optimizations     - Run benchmarks with different optimization levels"
	@echo "  test                      - Run unit tests"
	@echo "  clean                     - Clean build artifacts"
	@echo "  help                      - Show this help message"
	@echo ""
	@echo "Requirements for plotting:"
	@echo "  - Python 3.x with matplotlib and pandas: pip install matplotlib pandas"

.PHONY: all benchmark_optimized benchmark_debug quick_benchmark multi_type_benchmark multi_type_benchmark_debug run_quick_benchmark run_multi_type_benchmark run_benchmark run_all_optimizations test clean help