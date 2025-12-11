# Makefile for Dual-Pivot Quicksort Benchmark

CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -DNDEBUG -Ibenchmarks/include -Iinclude
BUILD_DIR = benchmarks/build
SRC_DIR = benchmarks/src
RUNNER = benchmarks/build/benchmark_runner.exe

# Default target
all: runner

# Build the benchmark runner
runner:
	if not exist benchmarks\build mkdir benchmarks\build
	$(CXX) $(CXXFLAGS) -o $(RUNNER) $(SRC_DIR)/benchmark_runner.cpp

# Run the benchmark manager
run: runner
	cd benchmarks && python benchmark_manager.py

# Clean build artifacts
clean:
	if exist benchmarks\build rmdir /s /q benchmarks\build
	if exist benchmarks\results\raw del /q benchmarks\results\raw\*

.PHONY: all runner run clean







# Clean build artifacts
clean:
	rm -rf $(RESULTS_DIR)\\*

# Help target
help:
	@echo "Available targets:"
	@echo "  all                       - Build optimized benchmark (default)"
	@echo "  benchmark_optimized       - Build with -O3 optimization"
	@echo "  benchmark_debug           - Build with debug symbols"
	@echo "  quick_benchmark           - Build quick demo benchmark"
	@echo "  multi_type_benchmark      - Build multi-type analysis benchmark"
	@echo "  multi_type_benchmark_debug - Build multi-type benchmark with debug"
	@echo "  full_benchmark            - Build comprehensive full benchmark (10,800 test combinations)"
	@echo "  run_quick_benchmark       - Run quick benchmark with plots (recommended)"
	@echo "  run_multi_type_benchmark  - Run multi-type analysis and generate plots"
	@echo "  run_full_benchmark        - Run comprehensive benchmark (43,200 algorithm tests)"
	@echo "  run_benchmark             - Run benchmark suite, generate CSV and plots"
	@echo "  run_all_optimizations     - Run benchmarks with different optimization levels"
	@echo "  test                      - Run unit tests"
	@echo "  clean                     - Clean build artifacts"
	@echo "  help                      - Show this help message"
	@echo ""
	@echo "Benchmark Information:"
	@echo "  full_benchmark tests 12 patterns × 18 data types × 50 array sizes × 4 algorithms"
	@echo "  Expected runtime: 30-60 minutes, generates ~10,800 CSV rows"
	@echo ""
	@echo "Requirements for plotting:"
	@echo "  - Python 3.x with matplotlib and pandas: pip install matplotlib pandas"

.PHONY: all benchmark_optimized benchmark_debug quick_benchmark multi_type_benchmark multi_type_benchmark_debug full_benchmark run_quick_benchmark run_multi_type_benchmark run_full_benchmark run_benchmark run_all_optimizations test clean help