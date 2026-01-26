# Empirical Constant Tuning Report

## Objective
To empirically determine the optimal values for key algorithm constants on the target hardware (Linux/WSL).

## Methodology
- **Strategy**: Coordinate Descent (tuning one parameter at a time).
- **Tool**: `benchmarks/tune_constants.py`
- **Metric**: Execution time (ms) averaged over 10 iterations.
- **Environment**: Linux environment, GCC C++20, -O3 optimization.

## Experiment 1: MAX_INSERTION_SORT_SIZE
**Definition**: The threshold below which Dual-Pivot Quicksort switches to Insertion Sort.
**Workload**: Sequential sort, 1,000,000 integers.
**Range**: 10 to 80 (step 5).

### Results
| Threshold | Time (ms) | Notes |
|-----------|-----------|-------|
| 10 | 45.83 | |
| 15 | 46.45 | |
| 20 | 45.67 | |
| 25 | 46.16 | |
| 30 | 45.89 | |
| 35 | 45.52 | Good |
| 40 | 45.71 | |
| 45 | 45.26 | Very Good |
| 50 | 45.40 | |
| **55** | **45.25** | **Optimal** |
| 60 | 45.34 | Good |
| 65 | 46.12 | |
| 70 | 46.22 | |
| 75 | 49.16 | Significant degradation |
| 80 | 47.05 | |

**Analysis**:
The performance curve is relatively flat between 35 and 60. The absolute minimum was observed at **55** (45.25 ms). Values above 70 start to degrade performance significantly, likely due to the $O(N^2)$ nature of insertion sort overtaking the overhead savings.

## Experiment 2: MIN_PARALLEL_SORT_SIZE
**Definition**: The array size threshold below which parallel execution switches to sequential.
**Workload**: Parallel sort (4 threads), 10,000,000 integers.
**Range**: 1024 to 65536 (log scale).

### Results
| Threshold | Time (ms) | Notes |
|-----------|-----------|-------|
| 1024 | 146.74 | High overhead |
| 2048 | 120.75 | |
| 4096 | 112.68 | Previous Default |
| 8192 | 121.15 | Noise? |
| 16384 | 125.87 | |
| 32768 | 112.36 | |
| **65536** | **111.31** | **Optimal** |

**Analysis**:
Lower thresholds (1024) cause excessive task creation overhead, hurting performance (146 ms). The "sweet spot" appears to be between 4096 and 65536. The optimal value found was **65536**, suggesting that for this machine, sorting chunks smaller than 65k elements is faster sequentially than incurring the overhead of a new task.

## Conclusion and Actions
1. **MAX_INSERTION_SORT_SIZE**: Updated from 32 to **55**.
2. **MIN_PARALLEL_SORT_SIZE**: Updated from 4096 to **65536**.
3. **Refactoring**: `include/dpqs/utils.hpp` was refactored to remove hardcoded constants and include `include/dpqs/constants.hpp`, enabling this tuning process.

## Update: Extended Range for Experiment 2
Based on initial findings, the range for `MIN_PARALLEL_SORT_SIZE` was extended to find the true inflection point.

**Extended Range**: 32,768 to 1,048,576 (log scale).

### Extended Results
| Threshold | Time (ms) | Notes |
|-----------|-----------|-------|
| 32768 | 112.36 | |
| **65536** | **111.31** | **Global Optimal** |
| 131072 | 114.47 | Performance degrades |
| 262144 | 132.25 | Significant degradation |
| 524288 | 148.83 | |
| 1048576 | 186.69 | Effectively sequential performance |

**Final Analysis**:
The extended test confirms that **65536** is indeed the optimal threshold. Increasing the threshold further (e.g., to 128k or 256k) degrades performance because it reduces the available parallelism too much for the workload size (10M elements). At 1M threshold (1/10th of the array), we likely don't create enough parallel tasks to saturate the cores.

**Confirmed Action**:
Retain **65536** as the optimal `MIN_PARALLEL_SORT_SIZE`.
