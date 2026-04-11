# Chapter 4: Parallel Execution

This chapter presents the parallel implementation: from the work-stealing thread pool design through performance tuning to understanding fundamental hardware limits on parallel speedup.

---

## 4.1 Prior Work: Parallel Sorting and Work-Stealing

### 4.1.1 The Work-Stealing Paradigm

Blumofe and Leiserson (1999) introduced work-stealing as a solution for parallelizing recursive algorithms. The key insight: recursive decomposition creates imbalanced work trees where static thread assignment leads to idle threads.

Work-stealing addresses this through dynamic load balancing:
- Each thread maintains a local task queue
- Idle threads "steal" tasks from busy threads' queues
- Theoretical guarantee: O(T1/P + T_inf) expected time with P processors

The access pattern is critical:
- **LIFO local access**: Threads pop their most recent task (likely still in L1 cache)
- **FIFO stealing**: Thieves take the oldest task (largest partition, near recursion root)

### 4.1.2 Existing Implementations

| System | Key Feature | Limitation |
|--------|-------------|------------|
| Intel TBB | Auto-partitioner | External library dependency |
| Java ForkJoinPool | CountedCompleter pattern | JVM-specific |
| C++ `std::async` | Simple API | No work-stealing; creates threads per call |

**Table 4.1**: Parallel task systems evaluated.

Our implementation draws from Java's `ForkJoinPool` work-stealing design, but uses a simpler completion model: a global atomic counter instead of Java's hierarchical `CountedCompleter`. This simplification is appropriate because sorting is "fire-and-forget"—sorted partitions don't return values to parents.

### 4.1.3 The Memory Wall

Wulf and McKee (1995) identified that CPU speed grows faster than memory bandwidth—the "memory wall." Sorting is memory-bound: data movement dominates computation. This implies parallel speedup is limited by shared memory bandwidth, not CPU count. Our experiments confirm this: beyond 8 threads, L3 cache contention becomes the dominant bottleneck.

---

## 4.2 The Problem

Recursive dual-pivot sorting creates inherently imbalanced work:
1. Each partition divides into three unequal regions
2. Partition quality varies with pivot selection luck
3. Static thread assignment leaves threads idle when their assigned region completes early

The challenge: achieve dynamic load balancing without a centralized bottleneck that serializes task distribution.

---

## 4.3 Design Evolution

The thread pool underwent three major iterations, each addressing a specific bottleneck.

### 4.3.1 Version 1: Blocking Parent

The initial implementation used `std::future` with blocking `.get()` calls. Parent tasks submitted children and waited for completion.

**The Thread Starvation Problem**: Parent threads hold pool slots while waiting. To sort with 16 leaf tasks (depth 4), we need:
- 16 threads for leaf work
- 15 threads waiting (internal nodes: 1 + 2 + 4 + 8)
- **Total: 31 threads required**, but only 24 available

**Result**: Performance plateaued at 2-4 threads. Adding more threads made it *slower*.

### 4.3.2 Version 2: Fire-and-Forget

Shifted to task-based parallelism without blocking:
- **No futures**: Tasks don't return values to parents
- **Quiescence detection**: Global `std::atomic<int>` tracks active tasks
- **Tail call optimization**: Parent processes one partition directly instead of spawning all three

| Threads | Time (s) | Speedup |
|---------|----------|---------|
| 1 | 3.22 | 1.00x |
| 2 | 1.83 | 1.76x |
| 4 | 1.08 | 2.98x |
| 8 | 0.76 | 4.24x |
| 16 | 0.65 | 4.95x |

**Table 4.2**: Version 2 performance on 50M integers.

**New Bottleneck**: Mutex contention on the global task queue at >16 threads.

### 4.3.3 Version 3: Work-Stealing with Distributed Queues

To eliminate global mutex contention:
- **Distributed queues**: Each thread owns a local `WorkStealingQueue`
- **LIFO local access**: Pop from back (most recent task, likely in cache)
- **FIFO stealing**: Steal from front (oldest = largest partitions)
- **`try_lock`**: Non-blocking steal attempts; no spinning on contention

**Result**: Achieved 5.18x speedup on 16 threads for 10M elements.

---

## 4.4 Implementation Design

### 4.4.1 Work-Stealing Queue

The `WorkStealingQueue` (`threadpool.hpp`) uses `std::deque` protected by a mutex, with separate access patterns for owners and thieves:

| Access | Direction | Rationale |
|--------|-----------|-----------|
| Local pop | Back (LIFO) | Most recent task likely in L1/L2 cache |
| Steal | Front (FIFO) | Oldest task is largest (near recursion root) |

**Table 4.3**: Queue access patterns.

The `try_to_lock` mechanism makes stealing non-blocking: if a queue's mutex is held, the thief immediately moves to another victim rather than waiting.

### 4.4.2 Victim Selection: Sticky Victim

When a thread's local queue is empty, it must select a victim to steal from. Naive round-robin scanning wastes cycles. The sticky victim strategy remembers the last successful victim:
- If thread T successfully steals from thread V, likely V has more tasks from the same subtree
- Same subtree means spatially close data in memory -> better cache locality
- Avoids randomly scanning all queues on each steal attempt

---

## 4.5 Performance Tuning

### 4.5.1 Task Granularity

The threshold `MIN_PARALLEL_SORT_SIZE` controls when to spawn parallel subtasks versus sorting sequentially. This involves a fundamental trade-off:
- **Lower threshold** -> More tasks -> Better load balancing, but higher L3 contention
- **Higher threshold** -> Fewer tasks -> Less contention, but worse load balancing

**VTune-guided analysis** on 10M integers:

| Threshold | Tasks | L3 Bound (16T) | Runtime 4T | Runtime 16T |
|-----------|-------|----------------|------------|-------------|
| 8,192 | ~1,220 | 30.9% | 145 ms | 105 ms |
| 16,384 | ~610 | ~20% | 126 ms | 108 ms |
| 32,768 | ~305 | ~15% | 112 ms | 124 ms |
| 65,536 | ~153 | 18.1% | 111 ms | 106 ms |

**Table 4.4**: Threshold sweep with VTune profiling.

The sweep revealed a **bimodal performance pattern**:

| Zone | Threshold | Mechanism |
|------|-----------|-----------|
| Parallelism-optimal | 8k-12k | Many tasks mask L3 contention via parallelism |
| Dead zone | 20k-40k | Too few tasks for load balancing; still enough for cache thrashing |
| Cache-optimal | 50k-65k | Low contention but limited parallelism |

**Table 4.5**: Performance zones by threshold.

**Design decision**: `MIN_PARALLEL_SORT_SIZE = 8192`. This provides optimal performance at recommended thread counts (4-8) while remaining competitive at higher counts.

### 4.5.2 Practical Thread Recommendations

| Threads | Speedup | Efficiency | L3 Bound | Recommendation |
|---------|---------|------------|----------|----------------|
| 4 | 3.30x | 82% | 6.8% | Good default |
| 8 | 4.62x | 58% | 19% | Production use |
| 16 | 5.18x | 32% | 38% | Diminishing returns |

**Table 4.6**: Thread scaling characteristics.

Exhausting all CPU cores is not recommended:
1. **Diminishing returns**: 4T->8T gains +1.32x speedup; 8T->16T gains only +0.56x
2. **System responsiveness**: Other processes degrade
3. **Energy efficiency**: 16T uses ~2x power for only 12% more speedup vs 8T

**Recommendation**: Use <=50% of system threads for batch sorting jobs.

### 4.5.3 Cache-Line Padding

Modern CPUs transfer data in 64-byte cache lines. When threads modify variables on the same cache line, the MESI coherency protocol causes "false sharing"—each write invalidates other cores' copies, forcing expensive L3 fetches.

The implementation uses `alignas(64)` to isolate each queue on its own cache line. Synthetic microbenchmarks show false sharing causes 4-13x slowdown in high-contention scenarios.

**Honest assessment**: In our sorting workload, this optimization provides negligible measured benefit because:
- Only ~1,000 task operations occur for 10M elements (vs. millions of element accesses)
- The heap allocator naturally separates queue allocations
- Array data movement dominates L3 contention, not queue metadata

We retain `alignas(64)` as defensive practice: it costs nothing (compile-time directive), follows industry consensus (Intel, Java, Microsoft all recommend it), and future-proofs against higher-contention workloads.

---

## 4.6 Chapter Summary

| Component | Design Choice | Impact |
|-----------|---------------|--------|
| Work-stealing | Distributed queues with LIFO local / FIFO steal | Eliminates global mutex bottleneck |
| Completion | Global atomic counter (`incomplete_tasks`) | Simple quiescence detection |
| Victim selection | Sticky victim | Improves cache locality during stealing |
| Granularity | `MIN_PARALLEL_SORT_SIZE = 8192` | Optimal load balancing at 4-8 threads |
| Cache isolation | `alignas(64)` on queues | Defensive best practice |

**Table 4.7**: Summary of parallel design choices.

The implementation achieves 5.18x speedup at 16 threads and 4.62x at 8 threads on 10M integers. The limiting factor is not the thread pool design but memory bandwidth: sorting 40MB of data saturates L3 cache capacity regardless of parallelism. Chapter 5 presents experimental evaluation confirming these architectural limits.
