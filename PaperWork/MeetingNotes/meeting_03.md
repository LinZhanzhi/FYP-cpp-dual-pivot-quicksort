# Meeting Note 3

**Date:** 2025-12-03
**Time:** 14:00 - 14:45
**Location:** Supervisor's Office (PQ706)
**Attendees:** LIN Zhanzhi, CAO Yixin (Supervisor)

## 1. Objectives
*   Review the draft of the Interim Report.
*   Analyze preliminary benchmarking results.
*   Discuss requirements for the Interim Presentation Video.

## 2. Progress Report
*   **Interim Report:** Completed 80% of the report (Introduction, Literature Review, Methodology).
*   **Implementation:** Refactored code to be fully generic using C++20 Concepts. Added `BenchmarkRunner` class.
*   **Results:** Initial benchmarks show DPQS is faster than `std::sort` for large arrays (>1M elements) of random integers.

## 3. Discussion & Feedback
### Key Discussion Points
*   **Report Structure:** Supervisor reviewed the draft. Pointed out that the "Methodology" section was too descriptive of the code. It needs to explain the *algorithm logic* more clearly (pseudocode/flowcharts).
*   **Benchmark Noise:** The current results have high variance.
    *   *Advice:* Run benchmarks on a dedicated machine or ensure background processes are minimized. Use "median" instead of "average" to filter outliers.
*   **Future Work:** Discussed the plan for Semester 2. Supervisor strongly suggested exploring **Parallelization** (using `std::thread` or OpenMP) as the next major step.

### Supervisor Feedback
> "The results look promising, but a table of numbers is hard to read. Generate line graphs showing 'Time vs Input Size'. Also, include a 'Speedup' graph relative to std::sort."

> "For the presentation video, make sure you *show* the sorting happening (maybe a small visualization or live demo of the benchmark running). Don't just read slides."

## 4. Issues & Challenges
*   **Small Array Performance:** DPQS is slower than `std::sort` for small arrays (<100 elements).
    *   *Resolution:* This is expected. Discussed implementing a hybrid approach (switching to Insertion Sort for small partitions), similar to Introsort. This will be a key task for the next phase.

## 5. Action Items
- [ ] Generate graphs for the Interim Report results section. (Due: 2025-12-10)
- [ ] Record and edit the Interim Presentation Video. (Due: 2025-12-15)
- [ ] Finalize and submit the Interim Report. (Due: 2025-12-18)
- [ ] Begin research on Parallel Quicksort strategies. (Due: Jan 2026)

## 6. Next Meeting Plan
*   **Target Date:** 2026-01-15 (Semester 2)
*   **Focus:** Parallel implementation plan and feedback on Interim Assessment.
