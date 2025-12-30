# Implementation Plan: Work Stealing Parallelism (V3)

**Status:** Proposed / Future Work
**Goal:** Eliminate global mutex contention to enable linear scaling beyond 24 threads.

## 1. Core Architecture: Distributed Queues

The central innovation is moving from a **Single Global Queue** to **Per-Thread Local Queues**.

### 1.1 The Double-Ended Queue (Deque)
Each thread owns a `std::deque<Task>`.
*   **Owner Access (LIFO):** The thread pushes and pops from the **bottom** (tail). This acts like a stack, preserving cache locality.
*   **Thief Access (FIFO):** Other threads steal from the **top** (head). This grabs the oldest (largest) tasks, maximizing the value of the steal.

### 1.2 Data Structure Design
```cpp
struct WorkStealingQueue {
    std::deque<Task> q;
    std::mutex mtx; // Protects ONLY this specific queue

    // Push a task to the bottom (Owner only)
    void push(Task t) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push_back(std::move(t));
    }

    // Pop from bottom (Owner only)
    bool try_pop(Task& t) {
        std::lock_guard<std::mutex> lock(mtx);
        if (q.empty()) return false;
        t = std::move(q.back());
        q.pop_back();
        return true;
    }

    // Steal from top (Thieves only)
    bool try_steal(Task& t) {
        // CRITICAL: Use try_lock to avoid blocking on contention
        std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
        if (!lock || q.empty()) return false;
        t = std::move(q.front()); // Steal oldest task
        q.pop_front();
        return true;
    }
};
```

---

## 2. The Worker Loop Logic

Each worker thread executes the following loop continuously:

1.  **Local Work (Fast Path):**
    *   Try to `pop()` from its *own* queue.
    *   If successful, execute the task immediately.
    *   This path is taken 99% of the time in a balanced workload.

2.  **Steal Work (Slow Path):**
    *   If the local queue is empty, become a "Thief".
    *   Select a **random victim** thread ID.
    *   Try to `steal()` from that victim.
    *   If successful, execute the task.

3.  **Retry Policy (Spinning vs. Yielding):**
    *   **Immediate Retry:** If a steal fails (victim empty or locked), immediately pick a new random victim. Do not wait.
    *   **Backoff:** Only after $ failed attempts (e.g., =2 \times \text{thread\_count}$), call `std::this_thread::yield()` to prevent CPU burning.

---

## 3. Addressing Edge Cases

### 3.1 The Bootstrap Problem (Ignition)
*   **Scenario:** At =0$, only Thread 0 has work (the full array). Threads 1-23 are empty.
*   **Solution:** Threads 1-23 immediately enter the "Steal Work" phase. They randomly probe queues. Within microseconds, one will find Thread 0, steal half the work, and become a new source. The work spreads exponentially ( \to 2 \to 4 \to 8 \dots$).

### 3.2 The Thundering Herd (Startup Contention)
*   **Scenario:** All 23 threads try to steal from Thread 0 simultaneously.
*   **Solution:** The `try_steal` function uses `std::try_to_lock`.
    *   Thread 1 gets the lock.
    *   Thread 2 tries, fails immediately (no blocking), and moves to Thread 3.
    *   This prevents a "parking lot" of sleeping threads forming behind Thread 0.

---

## 4. Advanced Optimization (Chase-Lev)
For maximum performance (C++20/Expert Level), the `std::mutex` can be removed entirely using the **Chase-Lev Deque** algorithm.
*   Uses `std::atomic` indices for `top` and `bottom`.
*   Owner `push/pop` becomes wait-free (just atomic loads/stores).
*   Stealing uses `compare_exchange_weak` (CAS).
*   *Note:* This is significantly harder to implement correctly and should only be attempted after the mutex-based version is working.
