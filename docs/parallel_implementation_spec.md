# Parallel Implementation Specification: Non-Blocking Task Orchestration

## 1. System Architecture

The system moves from a "Blocking Parent" model (waiting on futures) to a **"Fire-and-Forget"** model with **Quiescence Detection**.

### Core Components
1.  **Global Task Queue:** A thread-safe queue holding `std::function<void()>` tasks.
2.  **Worker Pool:** A fixed set of threads (hardware_concurrency) that consume tasks.
3.  **Quiescence Detector:** A mechanism to determine when *all* work is finished (Queue is empty AND no threads are active).

## 2. Detailed Component Design

### 2.1 The ThreadPool Class

The `ThreadPool` will manage the workers and the termination state.

**State Variables:**
-   `std::queue<std::function<void()>> tasks`: The work queue.
-   `std::mutex queue_mutex`: Protects the queue.
-   `std::condition_variable cv`: Wakes up sleeping workers.
-   `std::atomic<int> active_workers`: Counts threads currently executing a task.
-   `std::atomic<bool> done`: Flag to signal system shutdown.

**Key Methods:**

1.  **`submit(Task t)`**:
    -   Lock mutex.
    -   Push `t` to `tasks`.
    -   Unlock mutex.
    -   `cv.notify_one()` (Wake up a worker).

2.  **`worker_loop()`**:
    -   Loop while `!done`:
        -   Lock mutex.
        -   Wait on `cv` while `tasks.empty()` and `!done`.
        -   If `done` and `tasks.empty()`, return.
        -   Pop task `t`.
        -   Unlock mutex.
        -   **`active_workers++`**
        -   Execute `t()`.
        -   **`active_workers--`**
        -   *Check Quiescence:* If `active_workers == 0` and `tasks.empty()`:
            -   Notify main thread (or set a promise).

### 2.2 The Sorting Logic (The Task)

To minimize queue overhead and maximize cache locality, we use **Iterative Self-Work** for the smallest subarray.

**Function:** `parallel_sort_task(Iterator low, Iterator high)`

**Logic:**
```cpp
void parallel_sort_task(Iterator low, Iterator high) {
    while (std::distance(low, high) > THRESHOLD) {
        // 1. Partition
        auto [p1, p2] = partition(low, high);

        // 2. Define Subarrays
        // Range 1: [low, p1)
        // Range 2: [p1+1, p2)
        // Range 3: [p2+1, high)

        // 3. Sort ranges by size (descending)
        // Let r1 = largest, r2 = medium, r3 = smallest

        // 4. Offload Largest & Medium
        ThreadPool::instance().submit([=]{ parallel_sort_task(r1.begin, r1.end); });
        ThreadPool::instance().submit([=]{ parallel_sort_task(r2.begin, r2.end); });

        // 5. Iterate on Smallest (Tail Call Optimization)
        // Instead of recursing, we update 'low' and 'high' to point to r3
        // and loop again. This keeps the current thread working on the
        // most cache-local data without queue overhead.
        low = r3.begin;
        high = r3.end;
    }

    // Base Case: Sequential Sort
    sequential_sort(low, high);
}
```

### 2.3 Orchestration Flow

**Step 1: Initialization**
-   Main thread calls `dual_pivot::sort(arr)`.
-   `ThreadPool` is initialized (if not already).

**Step 2: Bootstrapping**
-   Main thread submits the initial task: `submit([=]{ parallel_sort_task(arr.begin(), arr.end()); })`.

**Step 3: Participation (Optional but Recommended)**
-   Main thread joins the `worker_loop()` to help process tasks instead of sleeping.
-   This effectively makes the main thread "Worker 0".

**Step 4: Termination**
-   The system needs to know when to stop.
-   We can use a `std::promise<void>` associated with the sort operation.
-   When `active_workers == 0` and `queue.empty()`, the last worker sets the promise value.
-   Main thread (if waiting) wakes up.

## 3. Safety & Edge Cases

1.  **Race Condition on Termination:**
    -   *Scenario:* Worker A finishes (active--), Queue empty. Worker B is about to push.
    -   *Fix:* `active_workers` must track *intent* or be decremented *after* pushing.
    -   *Refined Logic:* The `active_workers` count covers the execution of the task. If a task spawns children, it pushes them *before* it returns.
    -   So:
        1. Worker A running (Active=1).
        2. Worker A pushes Child 1, Child 2. (Queue size=2).
        3. Worker A finishes (Active=0).
        4. System checks: Active=0? Yes. Queue empty? No. -> Continue.
    -   This is safe.

2.  **Queue Explosion:**
    -   If we push too many small tasks, memory usage spikes.
    -   *Fix:* The `THRESHOLD` (e.g., 10,000 elements) prevents this. We only push tasks that are large enough to justify the overhead.

3.  **Exception Handling:**
    -   If a sorting task throws, we must catch it, set an error flag, and cancel all other tasks to prevent deadlock.

## 4. Implementation Roadmap

1.  Modify `ThreadPool` in `include/dpqs/parallel/threadpool.hpp` to support the new `submit` and `worker_loop` logic.
2.  Create `include/dpqs/parallel/task_sort.hpp` implementing the iterative `parallel_sort_task`.
3.  Update `include/dpqs/parallel/parallel_sort.hpp` to use the new task-based system.
