# Meeting Note 07

**Date:** 2026-01-29
**Time:** 14:00 - 14:30
**Location:** Supervision Office / Online
**Attendees:** LIN Zhanzhi, CAO Yixin (Supervisor)

## 1. Objectives
*   Review the implementation of the "Adaptive Granularity" optimization (Grain Size Tuning).
*   Discuss the logic for dynamic threshold adjustment based on system load.
*   Plan the validation and benchmarking phase for this feature.

## 2. Progress Report
*   **Adaptive Granularity Implemented**:
    *   Modified `include/dpqs/parallel/threadpool.hpp` to expose a real-time metric of system load via `get_active_task_count()`.
    *   Updated `include/dpqs/parallel/parallel_sort.hpp` to implement the dynamic thresholding logic.
    *   The sorter now checks the number of active tasks; if the pool is saturated (> 4 * thread count), the sequential fallback threshold is doubled (e.g., 4096 -> 8192).
*   **Verification**:
    *   Compiled and ran a smoke test (`benchmark_runner_adaptive`) successfully on 1,000,000 integers.
    *   No stability regressions observed.

## 3. Discussion & Feedback
### Key Discussion Points
*   **Load Metric Selection**: Discussed why `incomplete_tasks` (queued count) is a sufficient proxy for system load compared to more complex metrics like CPU utilization.
*   **Threshold Scaling**: The decision to simplify the logic to a binary state (Normal vs. Saturated) rather than a linear scaling function was approved to minimize runtime calculation overhead.

### Supervisor Feedback
> Supervisor approved the lightweight implementation of the adaptive check. They emphasized that the overhead of the check itself (checking an atomic variable) must be negligible compared to the sorting work, which appears to be the case.
> Suggested ensuring that the "saturation limit" ( \times N$) is tunable or verified against different hardware configurations in future benchmarks.

## 4. Issues & Challenges
*   **Benchmarking Environment**: Need to ensure the test machine is quiet during the upcoming fine-grained benchmarks to detect the potentially subtle improvements in throughput. - [Status: Resolved]

## 5. Action Items
- [x] Implement Adaptive Granularity logic (Completed).
- [ ] Run full-scale comparative benchmarks (Static vs. Adaptive) on varying array sizes (10M, 100M). (Due: Next Meeting)
- [ ] Begin investigation into "Memory-Aware Scheduling" for the next optimization phase. (Due: Next Meeting)

## 6. Next Meeting Plan
*   **Target Date:** 2026-02-05
*   **Focus:** Review Benchmark Results & Memory-Aware Scheduling Plan.
