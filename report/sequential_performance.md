## 8.2 Performance Analysis: Sequential Algorithms

This section evaluates the performance of the implemented Dual-Pivot Quicksort against the C++ Standard Library `std::sort` (Introsort). The analysis is based on over 5,000 benchmark data points, covering array sizes from $N=1,000$ to $N=10,000,000$ across various data distributions.

### 8.2.1 Baseline Efficiency (Random Permutations)
On uniformly random data (RANDOM_SHUFFLE), the Dual-Pivot implementation demonstrates a consistent performance advantage over `std::sort` across the entire range of input sizes.

**Performance Observation:**
Contrary to initial expectations that `std::sort` might hold a marginal advantage for small $N$ due to simplicity, empirical results indicate that the Sequential Dual-Pivot implementation is faster for **all** tested input sizes ($10^3 \le N \le 10^7$).

**Key Results (Execution Time in ms):**
| Input Size ($N$) | Dual-Pivot Sequential | `std::sort` (Introsort) | Speedup |
| :--- | :--- | :--- | :--- |
| **1,000** | 0.018 ms | 0.026 ms | **+30%** |
| **100,000** | 3.89 ms | 4.46 ms | **+12%** |
| **1,000,000** | 45.49 ms | 53.80 ms | **+15%** |
| **10,000,000** | 553.96 ms | 627.65 ms | **+11%** |

As shown in **Figure 8.1** (Log-Log plot placeholder), the execution time scales linearly with $N \log N$ ($R^2 > 0.99$ for both). However, the Dual-Pivot Quicksort maintains a lower constant factor, likely attributed to:
1.  **Cache Efficiency**: Two pivots produce three segments, providing better locality of reference during partitioning.
2.  **Reduced Height**: Base-3 logarithms imply a shorter recursion depth compared to the base-2 recursion of standard QuickSort/Introsort.

**Result**: The custom implementation successfully outperforms the industry-standard `std::sort` for random permutations without incurring significant overhead, even at small sizes.

[Insert Figure 8.1 Here]
*Caption: Log-Log plot of Execution Time (ns) vs Input Size (N) for Random Data. The slope confirms O(N log N) scalability, with Dual-Pivot consistently lower (faster) than std::sort.*

### 8.2.2 Algorithmic Intelligence (Pattern Detection)
A critical requirement of the project was to improve performance on non-random real-world data. The implementation achieves this through an "Adaptive Run-Merging" strategy (verified in `run_merger.hpp`).

[Insert Figure 8.2 Here]
*Caption: Performance Comparison on Structured Data (Reverse, Sawtooth, Organ Pipe). Note the logarithmic scale emphasizing the order-of-magnitude improvements.*

**1. Ordered Data (Sorted and Reverse-Sorted)**
Both algorithms demonstrate adaptive capabilities, performing significantly faster on ordered data compared to the random baseline.

*   **Adaptive Behavior**: Unlike basic Quicksort implementations, neither algorithm treats ordered arrays as generic inputs. Both adapt, but their efficiencies vary by pattern.
*   **Reverse Sorted (Major Win)**: `dual_pivot::sort` achieves true $O(N)$ complexity here. By explicitly detecting the descending sequence, it reverses it in a single pass (0.53 ms for $N=10^6$). While `std::sort` is efficient (3.81 ms), it is still **7.3x slower** than the Dual-Pivot implementation.
*   **Nearly Sorted (Slight loss)**: For data with few inversions, `std::sort` proves slightly more efficient (20.03 ms vs 25.84 ms). Its low-overhead heuristics for local disorder outperform the heavier run-detection logic of our implementation in this specific case.

**2. Speedup on Structured Patterns**
The "Run Merger" extends beyond simple sorted arrays, effectively handling complex patterns like **Sawtooth** (multiple sorted runs) and **Organ Pipe** (ascending then descending).

[Insert Figure 8.3 Here]
*Caption: Execution Time vs Input Size for Organ Pipe Pattern. DPQS exhibits linear efficiency ($O(N)$), while `std::sort` struggles.*

[Insert Figure 8.4 Here]
*Caption: Execution Time vs Input Size for Sawtooth Pattern. `std::sort` adapts better than on Organ Pipe, but DPQS still dominates.*

*   **Organ Pipe ($N=10^6$)**: A massive **27x speedup** is observed (1.97 ms vs 53.91 ms). The algorithm correctly identifies the two dominant runs, whereas `std::sort` fails to exploit this structure maximally.
*   **Sawtooth ($N=10^6$)**: The implementation is **8x faster** (2.61 ms vs 20.99 ms). As observed in the figures, `std::sort` handles Sawtooth significantly better than Organ Pipe (21 ms vs 54 ms), likely due to the partial sorting present in the ramps. However, our implementation's generic merging strategy keeps the runtime near-linear, resulting in an 8x advantage.

### 8.2.3 Robustness (Duplicate Handling)
Handling arrays with low cardinality (few unique elements) is a known potential weakness for standard Quicksort implementations (fat partition problem). However, the benchmark results demonstrate that the Dual-Pivot implementation effectively neutralizes this issue.

[Insert Figure 8.5 Here]
*Caption: Effect of Duplicate Density on Execution Time ($N=10^6$). DPQS performance remains invariant compared to the Random baseline.*

**Invariant Performance:**
A key finding is the remarkable stability of the Dual-Pivot implementation across varying levels of duplication. As shown in **Figure 8.5**, the execution time for duplicate-heavy inputs is nearly identical to the random baseline.

*   **Random Data ($N=10^6$)**: 45.49 ms
*   **Many Duplicates (90%)**: 46.59 ms
*   **Many Duplicates (10%)**: 46.96 ms

**Comparison with `std::sort`:**
While `std::sort` also handles duplicates robustly (avoiding $O(N^2)$ degradation), the Dual-Pivot implementation maintains a consistent performance lead across all tested duplication levels.
*   **Lead Consistency**: Whether the duplicates are frequent (e.g., `MANY_DUPLICATES_10`) or sparse, DPQS remains approximately **7-15% faster** than `std::sort`, mirroring the advantage seen in random permutations.
*   **Mechanism**: This confirms that the 3-way partitioning logic successfully aggregates pivot-equal elements, preventing empty recursive calls and maintaining balanced partition sizes regardless of value cardinality.
