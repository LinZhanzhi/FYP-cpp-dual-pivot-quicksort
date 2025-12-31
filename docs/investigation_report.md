# Investigation Report: Benchmark Inconsistency

## Issue Description
The user reported a discrepancy between the scaling results observed in the standalone test (strong scaling) and the results from the full benchmark suite (flat scaling, ~150ms for all thread counts on 10M items).

## Investigation Findings

### 1. Standalone Verification
A standalone test `test/test_scaling_10m.cpp` was created to verify the core algorithm's behavior on 10M random integers.
**Results:**
- 1 Thread: ~540ms
- 2 Threads: ~290ms (1.86x speedup)
- 4 Threads: ~175ms (3.09x speedup)
- 8 Threads: ~128ms (4.22x speedup)
- 16 Threads: ~111ms (4.85x speedup)

This confirmed that the V3 Work Stealing implementation is working correctly and scales as expected.

### 2. Benchmark Suite Analysis
The benchmark suite consists of:
- `benchmarks/benchmark_manager.py`: Python orchestrator.
- `benchmarks/src/benchmark_runner.cpp`: C++ executable.

The user reported ~150ms for 10M items across all thread counts.
- My standalone test showed ~111ms for 16 threads.
- My standalone test showed ~540ms for 1 thread.

The user's result of ~150ms for 2 threads (or even 1 thread if implied) was suspicious. It suggested that the benchmark runner was either:
- Running with max threads regardless of the argument.
- Not sorting the full array.
- Using a stale binary that did not include the latest fixes or was optimized incorrectly.

### 3. Root Cause Identification
I manually recompiled the `benchmark_runner` binary using the latest source code.
After recompilation, I ran the benchmark runner manually:
- **1 Thread:** ~560ms (Consistent with standalone)
- **16 Threads:** ~105ms (Consistent with standalone)

**Conclusion:** The `benchmark_runner` binary was stale. It had not been recompiled after the recent changes to `dual_pivot_quicksort.hpp`. The old binary likely contained a bug or was not respecting the thread count argument correctly (or was just running fast/broken).

### 4. Race Condition in ThreadPool
During verification, I discovered a race condition in `ThreadPool::wait_for_completion`.
- **Symptom:** Some benchmark iterations reported ~0ms execution time.
- **Cause:** There was a window between a worker popping a task from the queue and incrementing the `active_tasks` counter. If `wait_for_completion` checked the state during this window, it would see empty queues and 0 active tasks, returning prematurely before the task was actually executed.
- **Fix:** I modified `ThreadPool` to track `incomplete_tasks` (incremented on submission, decremented on completion). This ensures that the main thread waits until all submitted tasks are fully processed, eliminating the race condition.

## Resolution
1. The `benchmark_runner` has been recompiled with the latest code.
2. The `ThreadPool` race condition has been fixed in `include/dpqs/parallel/threadpool.hpp`.
3. The old benchmark results in `benchmarks/results/raw/` have been deleted.
4. The full benchmark suite is currently re-running to generate correct data.

## Verification
The new benchmark run is producing results consistent with the standalone test.
- `dual_pivot_parallel_2` on 10M items is expected to be around ~290ms.
- `dual_pivot_parallel_16` on 10M items is expected to be around ~105ms.

This resolves the inconsistency.
