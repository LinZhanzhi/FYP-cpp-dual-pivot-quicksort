# Benchmark System Refactoring

## Directory Structure Refactoring

- **src**: Contains the C++ source code for the single-unit benchmark runner.
- **include**: Contains header files (`data_generator.hpp`, `timer.hpp`) and the `pattern/` directory.
- **results/raw**: Directory where individual CSV result files will be stored.

## Benchmark Runner (`benchmark_runner.cpp`)

- **Single Unit Test**: This executable runs a single benchmark configuration (Algorithm + Type + Pattern + Size).
- **CSV Output**: It outputs a single-line CSV file with the results (including the pattern name).
- **CLI Arguments**: Accepts `--algorithm`, `--type`, `--pattern`, `--size`, and `--output` arguments.
- **Fixes**: Fixed several compilation errors in `data_generator.hpp` (type mismatches in `std::min`) and `dual_pivot_quicksort.hpp` (template declaration issues) to ensure smooth compilation.

## Benchmark Manager (`benchmark_manager.py`)

- **Automation**: A Python script that generates all combinations of algorithms, types, patterns, and sizes.
- **Resumable**: It checks if the result file for a specific combination already exists in `results/raw`. If it does, it skips that test. This allows you to stop and restart the benchmark process at any time without losing progress.
- **Execution**: It calls the compiled `benchmark_runner` for each missing combination.

## How to Use

### 1. Build the Runner
First, compile the C++ benchmark runner. This is required for both the web interface and command-line usage.

```bash
# Using the Makefile (Recommended)
make runner

# Or manually
mkdir -p build
g++ -std=c++17 -O3 -I include -I ../include src/benchmark_runner.cpp -o build/benchmark_runner
```

### 2. Start the Web Interface (Recommended)
The easiest way to manage benchmarks is through the web interface.

1. Start the server:
   ```bash
   python3 server.py
   ```
2. Open your browser and navigate to:
   ```
   http://localhost:8000
   ```
3. From the web page, you can:
   - View the status of all benchmark tests.
   - Start, stop, or resume the benchmark process.
   - See real-time results.

### 3. Command Line Usage (Alternative)
You can also run the benchmarks directly from the terminal.

```bash
python3 benchmark_manager.py
```

- You can interrupt this script (Ctrl+C) at any time.
- Run it again, and it will resume from where it left off.

### 4. Analyze Results
The results are stored as individual CSV files in `results/raw`. You can easily merge them into a single CSV file for analysis using a simple command like:

```bash
# Example merging command (ensure to handle headers appropriately)
cat results/raw/*.csv > all_results.csv
```

The system is now robust and ready for extensive benchmarking.
