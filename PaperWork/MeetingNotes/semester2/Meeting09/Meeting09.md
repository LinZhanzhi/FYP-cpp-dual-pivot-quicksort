# Meeting Note 09

**Date:** 2026-02-12
**Time:** 14:00 - 14:30
**Location:** Supervision Office / Online
**Attendees:** LIN Zhanzhi, CAO Yixin (Supervisor)

## 1. Objectives
*   Review the implementation and results of the "Hybrid Parallelism" (Depth-Based Cutoff).
*   Discuss the scaling breakthrough (breaking the 4.4x plateau).
*   Address the performance regression observed in structured data patterns.

## 2. Progress Report
*   **Hybrid Parallelism Implemented**:
    *   Added a recursion depth check in `include/dpqs/parallel/parallel_sort.hpp`.
    *   **Logic**: If depth > 20, the thread switches immediately to `sort_sequential` for the entire subtree, bypassing the scheduler.
    *   **Tuning**: Reduced `MIN_PARALLEL_SORT_SIZE` from 64k to 8k to improve initial load balancing.
*   **Benchmark Breakthrough (10M Integers)**:
    *   **16 Threads**: **4.51x - 4.66x** Speedup (Up from 4.4x).
    *   Successfully squeezed more performance out of the saturated system by removing scheduler overhead for leaf tasks.
*   **Challenges**:
    *   Regressions observed in `REVERSE_SORTED` (0.57x) and `ORGAN_PIPE` (0.76x) patterns compared to the ultra-fast sequential baseline.

## 3. Discussion & Feedback
### Key Discussion Points
*   **Sequential Fallback Success**: The improvement confirms that the overhead of creating/locking tasks was a significant factor at 16 threads. By removing it for the deep leaves, we reduced bus contention.
*   **Structured Data Regression**: The supervisor pointed out that `REVERSE_SORTED` is handled in $O(N)$ by the sequential Dual-Pivot (it just reverses the run). Parallelizing it forces it into $O(N \log N)$ splitting, which is inherently slower. This is an expected trade-off.

### Supervisor Feedback
> "The 5% gain at 16 threads is significant because you are fighting against Amdahl's Law and physical hardware limits. It validates the Hybrid approach."
> Regarding the regression: "Do not view the performance drop on 'Reverse Sorted' as a failure. It is a known property of parallelizers. However, you should document it clearly in the report."

## 4. Issues & Challenges
*   **No Further Scaling**: It is unlikely we can push beyond ~4.6x on this specific hardware without a complete algorithm rewrite (e.g., changing the partitioning scheme itself to be branchless or SIMD-optimized), which is out of scope.

## 5. Action Items
- [x] Implement Hybrid Parallelism.
- [x] Benchmarking & Analysis.
- [ ] **Final Report Writing**: Begin merging the individual implementation reports into the Final Thesis structure.
- [ ] **Code Cleanup**: Ensure all debug comments are removed and headers are standardized.

## 6. Next Meeting Plan
*   **Target Date:** 2026-02-19
*   **Focus:** Final Report Skeleton Review.
