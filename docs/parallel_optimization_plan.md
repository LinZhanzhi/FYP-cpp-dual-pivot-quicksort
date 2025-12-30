# Parallel Optimization Plan: Non-Blocking Task-Based Design

## 1. Objective
To eliminate the "Blocking Parent" bottleneck in the current parallel implementation and maximize CPU utilization by ensuring threads are never idle waiting for child tasks.

## 2. The Core Problem
The current implementation uses a `future.get()` pattern:
1.  Thread A partitions the array.
2.  Thread A spawns Thread B and Thread C for sub-tasks.
3.  **Thread A waits** for B and C to finish.

This wastes Thread A. In a deep recursion tree, more threads are waiting than working.

## 3. Proposed Design: Fire-and-Forget with Work Stealing

The new design follows a "Fire-and-Forget" approach where the parent thread does not wait. Instead, it offloads work and immediately continues processing or becomes available for other work.

### 3.1 Algorithm Flow

1.  **Partition:** The current thread performs the Dual-Pivot partition, resulting in 3 subarrays: `left`, `middle`, `right`.
2.  **Task Offloading:**
    -   Identify the 2 largest subarrays.
    -   Push these 2 subarrays as new **Tasks** to a global `TaskQueue`.
    -   **Do not wait** for them.
3.  **Self-Work:**
    -   The current thread immediately processes the 3rd (smallest) subarray itself (recursion).
    -   **Rationale:** Processing the smallest subarray locally is superior because:
        -   **Parallelism Propagation:** Large subarrays are "work generators." Pushing them to the global queue allows idle threads to pick them up and split them further, rapidly saturating the thread pool. If the current thread kept the largest subarray, it would bottleneck the creation of new tasks.
        -   **Load Balancing:** The current thread finishes the small task quickly and returns to the pool to assist others. If it kept the large task, it would be unavailable for a long time, potentially leaving other threads idle if the queue empties.
4.  **Completion:**
    -   When the current thread finishes its small subarray, it does **not** return to a parent (because the parent isn't waiting).
    -   Instead, it goes to the `TaskQueue` and pops a new task to execute.
5.  **Termination:**
    -   The entire sort is complete when the `TaskQueue` is empty AND all threads are idle. We use an atomic `active_tasks` counter to detect this.

### 3.2 Task Structure

```cpp
struct SortTask {
    Iterator start;
    Iterator end;
    // ... other context ...
};
```

### 3.3 Thread Pool Logic

The Thread Pool will no longer return `std::future`. It will be a void-return system.

```cpp
void worker_loop() {
    while (true) {
        SortTask task;
        if (queue.try_pop(task)) {
            atomic_active_tasks++;
            execute_sort(task);
            atomic_active_tasks--;
        } else {
            if (atomic_active_tasks == 0) break; // All done
            std::this_thread::yield(); // Or wait on condition variable
        }
    }
}
```

### 3.4 Threshold Optimization

To avoid overhead from too many small tasks:
1.  **Size Threshold:** If a subarray size < `MIN_PARALLEL_SIZE` (e.g., 10,000), sort it sequentially immediately. Do not push to queue.
2.  **Queue Limit:** If the `TaskQueue` is full (or has > 4 * num_threads tasks), do not push. Process all 3 subarrays recursively in the current thread. This prevents memory explosion.

## 4. Implementation Steps

1.  **Refactor ThreadPool:** Remove `std::future` dependency for internal tasks. Add `active_task_count`.
2.  **Create Task Struct:** Define a lightweight structure to hold sorting ranges.
3.  **Rewrite `parallelQuickSort`:**
    -   Remove `future.get()`.
    -   Implement the "Push 2, Run 1" logic.
    -   Add termination detection (Quiescence).

## 5. Expected Outcome
-   **Linear Scaling:** Threads will always be busy sorting or partitioning, never waiting.
-   **Reduced Overhead:** Processing the smallest subarray locally keeps the cache hot.
-   **Hardware Saturation:** We can fully utilize all 24 threads on the test machine without deadlocks or starvation.
