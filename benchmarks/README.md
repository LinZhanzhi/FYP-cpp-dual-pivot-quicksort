# Benchmark System Refactoring

## Directory Structure Refactoring

- **src**: Contains the C++ source code for the single-unit benchmark runner.
- **include**: Contains header files (`data_generator.hpp`, `timer.hpp`) and the `pattern/` directory.
- **legacy**: Archived all previous benchmark `.cpp` files here to keep the workspace clean.
- **results/raw**: Directory where individual CSV result files will be stored.

## Benchmark Runner (`benchmark_runner.cpp`)

- **Single Unit Test**: This executable runs a single benchmark configuration (Algorithm + Type + Pattern + Size).
- **Supported Algorithms**: Now supports `std_sort`, `dual_pivot`, `std_stable_sort`, and C standard `qsort`.
- **CSV Output**: It outputs a single-line CSV file with the results (including the pattern name).
- **CLI Arguments**: Accepts `--algorithm`, `--type`, `--pattern`, `--size`, and `--output` arguments.
- **Fixes**: Fixed several compilation errors in `data_generator.hpp` (type mismatches in `std::min`) and `dual_pivot_quicksort.hpp` (template declaration issues) to ensure smooth compilation.

## Benchmark Manager (`benchmark_manager.py`)

- **Automation**: A Python script that generates all combinations of algorithms (`std_sort`, `dual_pivot`, `std_stable_sort`, `qsort`), types, patterns, and sizes.
- **Resumable**: It checks if the result file for a specific combination already exists in `results/raw`. If it does, it skips that test. This allows you to stop and restart the benchmark process at any time without losing progress.
- **Execution**: It calls the compiled `benchmark_runner` for each missing combination.

## How to Use

### 1. Build the Runner
I've verified that `g++` is available. Run the following command to compile the runner:

```bash
mkdir -p build
g++ -std=c++17 -O3 -I include -I ../include src/benchmark_runner.cpp -o build/benchmark_runner
```

### 2. Run the Benchmarks
Start the manager script. It will automatically create the results directory and start running tests.

```bash
python3 benchmark_manager.py
```

- You can interrupt this script (Ctrl+C) at any time.
- Run it again, and it will resume from where it left off.

### 3. Analyze Results
The results are stored as individual CSV files in `results/raw`. You can easily merge them into a single CSV file for analysis using a simple command like:

```bash
# Example merging command (ensure to handle headers appropriately)
cat results/raw/*.csv > all_results.csv
```

The system is now robust and ready for extensive benchmarking.
