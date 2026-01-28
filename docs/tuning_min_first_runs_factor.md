# Tuning Report: MIN_FIRST_RUNS_FACTOR

**Date:** January 28, 2026
**Component:** Dual-Pivot Quicksort (Run Merger Heuristic)

## 1. Objective
The  constant controls the heuristic used to decide whether an array is "structured enough" to switch from the default Dual-Pivot Quicksort strategy to a timsort-style Run Merger.

The check is implemented as:
```cpp
// count: number of runs found so far
// k - low: total elements scanned so far
if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
    // Too many runs (average run length is too small) -> Abort merge, switch to Quicksort
    return false;
}
```

Mathematically, this enforces: $\text{Average Run Length} \ge 2^{\text{FACTOR}}$.

*   **Original Value:** 7 (^7 = 128$).
*   **Goal:** Determine the precise "break-even" run length where Merging becomes faster than Quicksort, and adjust the factor to capture that optimization window.

## 2. Methodology
We implemented a benchmark () that generates arrays of 10,000,000 integers composed of pre-sorted "runs" of fixed length $.

We compared two forced modes:
1.  **Force Merge**: Configuration where the heuristic always accepts validity (Factor = 1).
2.  **Force Quicksort**: Configuration where the heuristic always rejects (Factor = 30).

**System:** Linux Workstation (10M integers,  generated data).

## 3. Results

The following table compares execution time (in milliseconds) for sorting 10M integers with specific run characteristics.

| Run Length ($) | Force Merge (ms) | Force Quicksort (ms) | Winner |
| :--- | :--- | :--- | :--- |
| **32** | 82 | **78** | **Quicksort** (+5%) |
| **64** | **75** | 77 | **Merge** (+2.6%) |
| **80** | **69** | 69 | Tie |
| **96** | **60** | 61 | **Merge** (+1.6%) |
| **128** | **58** | 59 | **Merge** (+1.7%) |

### Analysis
*   **At =32*: The overhead of managing many small runs in the Merger outweighs the benefit of skipping sorting. Quicksort is faster.
*   **At =64*: The Merger becomes more efficient than Quicksort. The cost of merging reduces as the number of runs decreases.
*   **At =128*: The Merger is consistently superior.

## 4. Tuning Decision

The crossover point lies between run lengths of 32 and 64.

*   **Old Value (7)**: Required  \ge 128$. This was too conservative, missing the optimization window for run lengths between 64 and 128 (where Merging is already faster).
*   **New Value (6)**: Requires  \ge 64$.
    *   Captures the win at =64$ (75ms vs 77ms).
    *   Avoids the regression at =32$.
    *   Safely defaults to Quicksort for random data (Average run length $\approx 2$).

**Action:** Updated  from **7** to **6**.
