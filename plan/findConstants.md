# Automated Constant Tuning Plan

## 1. Objective
To empirically determine the optimal values for the heuristic thresholds (constants) in the Dual-Pivot Quicksort (DPQS) implementation for the specific target hardware. This ensures the algorithm adapts to the machine's cache size, branch prediction capabilities, and thread creation overhead.

## 2. Target Constants
The following constants in `include/dpqs/constants.hpp` govern key algorithmic decisions:

| Category | Constant | Default | Description |
| :--- | :--- | :--- | :--- |
| **Base Case** | `MAX_INSERTION_SORT_SIZE` | 44 | Threshold to switch from Quicksort to simple Insertion Sort. |
| | `MAX_MIXED_INSERTION_SORT_SIZE` | 65 | Threshold to switch to Mixed Insertion Sort (Sentinels). |
| **Parallelism** | `MIN_PARALLEL_SORT_SIZE` | 4096 | Minimum array size required to spawn a new thread/task. |
| **Adaptive** | `MIN_TRY_MERGE_SIZE` | 4096 | Minimum size to attempt run detection (adaptive sort). |
| | `MIN_FIRST_RUN_SIZE` | 16 | Minimum length of the first run to continue searching for structure. |
| **Counting** | `COUNTING_SORT_THRESHOLD_BYTE` | 29 | Threshold for byte counting sort. |

## 3. Methodology: "Coordinate Descent"
Since an exhaustive grid search of all constants simultaneously is computationally infeasible, we will optimize them sequentially in order of impact (Dependency Order).

**Optimization Order:**
1.  **Leaf Optimization (Base Cases):** These are executed most frequently (millions of times per large sort). Optimizing `MAX_INSERTION_SORT_SIZE` affects the efficiency of *every* QuickSort partition.
2.  **Parallel Thresholds:** Once the sequential leaf performance is stable, we tune `MIN_PARALLEL_SORT_SIZE` to balance thread management overhead against parallelism gains.
3.  **Adaptive Thresholds:** Finally, we tune when the algorithm checks for pre-sorted patterns to ensure the check itself isn't too costly for random data.

## 4. Implementation Plan

### Step 1: Parameterization (Refactoring)
Modify `include/dpqs/constants.hpp` to allow compiler-injected definitions. This enables us to change constants without editing source code manually.

**Example Change:**
```cpp
#ifndef MAX_INSERTION_SORT_SIZE
constexpr int MAX_INSERTION_SORT_SIZE = 44;
#endif
```

### Step 2: Tuning Harness (Python Script)
Create a Python script (`benchmarks/tune_constants.py`) that acts as the driver.

**Script Logic:**
1.  **Input:** Constant name, Range (Start, End, Step), Benchmark Configuration.
2.  **Loop:** For each value in Range:
    a.  **Clean:** `make clean -C benchmarks`
    b.  **Compile:** `make -C benchmarks CXXFLAGS="-DMAX_INSERTION_SORT_SIZE=X ..."`
    c.  **Benchmark:** Run `benchmark_runner` for specific sizes relevant to that constant.
    d.  **Record:** Parse the average runtime.
3.  **Output:** Plot runtime vs. Constant Value, identify the global minimum.

### Step 3: Execution Phases

#### Phase A: Base Case Tuning
*   **Target:** `MAX_INSERTION_SORT_SIZE`
*   **Range:** [10, 80], Step: 2
*   **Benchmark:** Sort 10 million random integers (repeated 10 times). The cumulative effect of base cases will show in the total runtime.
*   **Expectation:** A "U" shaped curve. Too small = too much recursion overhead. Too large = insertion sort's $O(n^2)$ dominates.

#### Phase B: Parallel Overhead Tuning
*   **Target:** `MIN_PARALLEL_SORT_SIZE`
*   **Range:** [1000, 32000], Step: Logarithmic (1k, 2k, 4k, 8k, 16k, 32k)
*   **Benchmark:** Sort large arrays (e.g., 50M integers) using all available cores.
*   **Expectation:** Finding the "sweet spot" where the cost of spawning a task is amortized by the speedup of parallel processing.

#### Phase C: Merging Heuristic
*   **Target:** `MIN_TRY_MERGE_SIZE`
*   **Range:** [256, 16384], Step: Powers of 2
*   **Benchmark:** Mixed bag of workloads (Random, Nearly Sorted, Reverse Sorted).
*   **Goal:** Ensure checking for runs doesn't slow down Random data (overhead check) while still catching Structured data early enough.

## 5. Deliverables
1.  **Refactored `constants.hpp`**: Ready for injection.
2.  **`tune_constants.py`**: The automation tool.
3.  **Final Report**: Used values vs. Optimal values found.
