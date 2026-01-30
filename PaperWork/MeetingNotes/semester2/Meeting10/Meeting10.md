# Meeting Note 10

**Date:** 2026-01-30
**Time:** 14:00 - 14:30
**Location:** Supervision Office / Online
**Attendees:** LIN Zhanzhi, CAO Yixin (Supervisor)

## 1. Objectives
*   Review the implementation and results of "Explicit Memory Management" (Ring Buffer optimization).
*   Discuss the unexpected performance regression.
*   Analyze the root cause and lessons learned.

## 2. Progress Report
*   **Explicit Memory Management Implemented**:
    *   Replaced `std::function<void()>` with a custom `SortTask` POD struct (64 bytes, cache-aligned).
    *   Implemented fixed-size Ring Buffers (capacity 8192) per worker thread, pre-allocated at pool initialization.
    *   Added inline comparator storage (`std::byte[24]`) to avoid heap allocation for stateful comparators.
    *   Updated `parallel_sort.hpp` and `completer.hpp` to use the new `enqueue_task(SortTask)` API.
*   **Benchmark Results (10M Integers, RANDOM)**:
    | Threads | Before (ms) | After (ms) | Change |
    |---------|-------------|------------|--------|
    | 2 | 258.78 | 272.49 | **+5.3% slower** |
    | 4 | 156.14 | 167.04 | **+7.0% slower** |
    | 8 | 116.73 | 122.65 | **+5.1% slower** |
    | 16 | 110.46 | 113.14 | **+2.4% slower** |
*   **Outcome**: The optimization resulted in a **3-7% performance regression** across most configurations.

## 3. Discussion & Feedback
### Key Discussion Points
*   **Why Did It Fail?**: Investigation revealed that `std::function` uses **Small Buffer Optimization (SBO)**. Our lambdas (~28 bytes captured state) fit within the SBO threshold, meaning no heap allocation was occurring in the original implementation.
*   **What Went Wrong**: The "fix" introduced additional overhead:
    1. Larger memory copies (64 bytes vs ~32 bytes for SBO)
    2. More complex circular buffer index arithmetic
    3. No actual reduction in allocations (there weren't any to begin with)
*   **Should We Revert?**: Discussed the trade-offs. Decided to keep the implementation for academic documentation value, as the investigation process and analysis are valuable learning outcomes.

### Supervisor Feedback
> "This is actually a good outcome for a thesis. You had a hypothesis, implemented it rigorously, measured the results, and discovered the hypothesis was wrong. That's the scientific method working correctly."
> "The lesson here is: always profile before optimizing. Assumptions about performance bottlenecks are often incorrect."
> "Document this clearly in your report as a 'Lessons Learned' section. Examiners appreciate honest analysis of failed optimizations."

## 4. Issues & Challenges
*   **Premature Optimization**: The implementation was based on theoretical analysis rather than actual profiling with tools like `perf` or `valgrind --tool=massif`.
*   **SBO Underestimated**: Modern `std::function` implementations are more efficient than commonly believed for small callables.

## 5. Action Items
- [x] Implement Explicit Memory Management (Ring Buffer + SortTask).
- [x] Run full benchmark suite to measure impact.
- [x] Analyze results and document root cause.
- [x] Write implementation report ([docs/reports/explicit_memory_management/report.md](../../docs/reports/explicit_memory_management/report.md)).
- [ ] Continue Final Report writing with this as a "Lessons Learned" case study.

## 6. Next Meeting Plan
*   **Target Date:** 2026-02-05
*   **Focus:** Final Report Progress Review, discuss remaining SIMD/Block-Based optimizations (if time permits).
