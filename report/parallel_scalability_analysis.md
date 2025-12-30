# Parallel Scalability Analysis: The "Blocking Parent" Bottleneck

## 1. Observation
Recent benchmarking of the Parallel Dual-Pivot Quicksort implementation revealed a performance plateau:
-   **2 Threads:** Significant speedup compared to sequential execution.
-   **4, 8, 16 Threads:** Performance remains almost identical to the 2-thread case, despite the test machine (Intel Core i7-13700) having 24 logical threads.

## 2. Root Cause Analysis
The limitation stems from the **"Blocking Parent"** concurrency pattern used in the current implementation () combined with a fixed-size thread pool.

### 2.1 The Blocking Mechanism
In the current  function, the code splits the work, enqueues new tasks, and then **immediately waits** for them to complete:

```cpp
auto future1 = pool.enqueue([=] { parallelQuickSort(...); });
auto future2 = pool.enqueue([=] { parallelQuickSort(...); });
future1.get(); // <--- BLOCKS the current worker thread
future2.get(); // <--- BLOCKS the current worker thread
```

When `future.get()` is called, the current thread (which is itself a worker from the thread pool) goes to sleep waiting for the result. It remains "occupied" in the thread pool but performs no useful CPU work.

### 2.2 Thread Starvation
This pattern consumes threads exponentially for *waiting* rather than *working*. To execute a parallel sort with a recursion depth of $, we need threads for both the leaf nodes (actual work) and the internal nodes (waiting parents).

For a binary split (simplified for explanation):
-   **Depth 0 (Root):** 1 thread waits.
-   **Depth 1:** 2 threads wait.
-   **Depth 2:** 4 threads wait.
-   **Depth 3:** 8 threads wait.

To execute a parallel sort with **16 leaf tasks** (Depth 4), we need:
-   **16 threads** for the actual work (leaves).
-   **15 threads** just to *wait* (internal nodes: 1+2+4+8).
-   **Total threads required:** 31 threads.

### 2.3 Hardware Limit
The test machine has **24 logical threads**.
-   At **2 threads** (Depth 1), we need 3 threads total. (Fits easily).
-   At **4 threads** (Depth 2), we need 7 threads total. (Fits easily).
-   At **8 threads** (Depth 3), we need 15 threads total. (Fits).
-   At **16 threads** (Depth 4), we need **31 threads**.

**The Bottleneck:** Since we only have 24 threads, we cannot simultaneously run all 16 tasks and hold all 15 waiting threads. The thread pool becomes saturated with waiting threads, forcing new tasks to sit in the queue until a slot opens up. This effectively **serializes** the execution of the remaining tasks, negating the benefit of requesting more threads.

## 3. Conclusion
The performance plateau is caused by thread pool saturation due to the lack of a non-blocking scheduler. The current implementation uses a blocking fork-join model where parent threads wait for children, consuming thread pool slots. As requested parallelism increases, the number of blocked threads exceeds the available hardware threads, forcing serialization.

To achieve linear scaling beyond this limit, the implementation must move to a **non-blocking** design where threads do not wait for children but instead return to the pool to process other tasks (Work Stealing or Fire-and-Forget).
