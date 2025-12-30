# Parallel Implementation Evolution Report

**Date:** December 30, 2025
**Subject:** Comparative Analysis of Parallel Dual-Pivot Quicksort Architectures

## Executive Summary
This report documents the evolution of the parallel sorting engine for the Dual-Pivot Quicksort project. We have implemented and tested two distinct architectures and identified a third for future development.

| Version | Architecture | Key Mechanism | Scaling Limit | Status |
| :--- | :--- | :--- | :--- | :--- |
| **V1** | Blocking Parent | `std::future` & `get()` | ~2-4 Threads | Deprecated |
| **V2** | Fire-and-Forget | Atomic Counters & Global Queue | ~16 Threads | **Active** |
| **V3** | Work Stealing | Distributed Deques | 24+ Threads | Proposed |

---

## 1. Version 1: The Blocking Parent (Naive Implementation)

### Architecture
The initial implementation relied on the C++ Standard Library's high-level concurrency primitives. It treated every recursive sort call as a function that returns a value (void, but synchronized).

*   **Mechanism:**
    *   Parent thread spawns 2 child tasks for the left and right partitions.
    *   Parent thread calls `future.get()` immediately after spawning.
    *   Parent thread **blocks** (sleeps) until children finish.

### Code Pattern
```cpp
// Pseudo-code of V1
auto left_future = pool.submit(sort, left_part);
auto right_future = pool.submit(sort, right_part);

// CRITICAL FLAW: Parent does nothing while waiting
left_future.get();  
right_future.get();
```

### Performance Analysis
*   **Bottleneck:** **Thread Starvation**.
    *   If we have 4 threads, and Thread 1 spawns 2 tasks, Thread 1 becomes useless while waiting.
    *   As recursion deepens, most threads end up stuck in a "waiting" state, holding onto their stack but doing no CPU work.
*   **Result:** Performance plateaued almost immediately. Adding more threads often made it *slower* due to overhead without utilization.

---

## 2. Version 2: Fire-and-Forget (Current Optimization)

### Architecture
The second iteration shifted to a task-based parallelism model that decouples task submission from task completion.

*   **Mechanism:**
    *   **Void Return:** Tasks do not return futures. The parent does not wait.
    *   **Quiescence Detection:** A global `std::atomic<int> active_tasks` counter tracks the total work in the system.
    *   **Tail Call Optimization:** The parent thread *becomes* the worker for the 3rd (middle) partition, avoiding a context switch.
    *   **Global Queue:** All threads push tasks to a single shared `std::queue` protected by a `std::mutex`.

### Code Pattern
```cpp
// Pseudo-code of V2
pool.push_task(sort, left_part);   // Fire
pool.push_task(sort, right_part);  // Fire

// Parent immediately continues working on the middle part
dual_pivot_quicksort(middle_part); // Forget (don't wait)
```

### Performance Analysis
*   **Improvement:** **High Utilization**. Threads are never blocked waiting for children. If a thread finishes its work, it goes back to the pool to grab a new task.
*   **Result:** Linear scaling up to ~16 threads on the i7-13700.
*   **New Bottleneck:** **Mutex Contention**. At >16 threads, the single lock protecting the global queue becomes a "hot spot," causing threads to fight for access rather than sorting.

---

## 3. Version 3: Work Stealing (Proposed Future Design)

### Architecture
To break the 16-24 thread limit, the next logical step is to remove the single point of contention (the global mutex).

*   **Mechanism:**
    *   **Distributed Queues:** Each thread has its own local `std::deque`.
    *   **Lock-Free Push/Pop:** A thread pushes and pops from its own queue with no locking (or very cheap synchronization).
    *   **Stealing:** When a thread runs out of work, it "steals" a task from the *back* of another thread's queue.

### Expected Benefits
*   **Zero Contention:** In the common case (plenty of work), threads never talk to each other.
*   **Cache Locality:** Threads process data they just generated, keeping it hot in L1/L2 cache.
*   **Scalability:** Capable of scaling to 64+ cores (Server-grade hardware).

---

## Summary of Transition

We successfully migrated from **V1** to **V2**, resulting in a **4.95x speedup** (from ~0.65s to ~0.13s for 100M integers). The move to **V3** is documented as the theoretical limit of the current hardware/software stack but is outside the scope of the current optimization sprint.
