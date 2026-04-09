# VTune-Guided Optimization Plan

**Date**: April 8, 2026
**Based on**: VTune Bottleneck Analysis
**Goal**: Improve performance across all thread counts (1-16)

---

## Executive Summary

VTune analysis revealed two distinct bottlenecks:

| Thread Count | Primary Bottleneck | Secondary |
|--------------|-------------------|-----------|
| 1-4 threads | Branch Misprediction (35%) | Front-End Bound |
| 8-16 threads | L3 Cache Contention (38%) | Synchronization (45%) |

This plan addresses both bottlenecks through incremental, measurable steps.

---

## Implementation Roadmap

```
┌─────────────────────────────────────────────────────────────────────┐
│                        OPTIMIZATION PHASES                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Step 1: Low-Risk Quick Wins (Solutions 3+5+6)                     │
│  ├── Increase MIN_PARALLEL_SORT_SIZE (8192 → 65536)                │
│  ├── Add cache line padding (alignas(64))                          │
│  ├── Add prefetch hints                                            │
│  └── Target: Reduce L3 contention at high thread counts            │
│      Expected: -15-25% time at 8-16 threads                        │
│                                                                     │
│  Step 2: Block-Based Partitioning (Solution 2)                     │
│  ├── Process elements in blocks of 64                              │
│  ├── Separate classification from data movement                    │
│  └── Target: Reduce branch misprediction at all thread counts      │
│      Expected: -15-25% time at 1-4 threads                         │
│                                                                     │
│  Step 3: Lock-Free Work Stealing (Solution 4) - OPTIONAL           │
│  ├── Replace mutex-based queues with Chase-Lev deques              │
│  └── Target: Eliminate synchronization overhead                    │
│      Expected: -20-30% time at 16 threads                          │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

# Step 1: Low-Risk Quick Wins

## Overview

| Solution | Risk | Effort | Target Bottleneck |
|----------|------|--------|-------------------|
| 3: Larger granularity | Low | 1 line | L3 cache contention |
| 5: Cache padding | Very Low | 5 lines | False sharing |
| 6: Prefetching | Low | 2 lines | Memory latency |

## Solution 3: Increase MIN_PARALLEL_SORT_SIZE

### Current Problem

```cpp
// constants.hpp (current)
constexpr int MIN_PARALLEL_SORT_SIZE = 8192;  // 32KB per task
```

With 10M elements and 8192 threshold:
- Number of tasks created: 10,000,000 / 8,192 ≈ **1,220 tasks**
- Threads constantly steal tasks from each other
- Each stolen task accesses different memory regions
- Result: L3 cache thrashing (37.9% L3 bound at 16 threads)

### Technical Details

```
Memory Access Pattern with Small Tasks:

Thread 0: [####]                    [####]        [####]     <- Jumps around
Thread 1:        [####]                   [####]             <- Jumps around
Thread 2:              [####]                         [####] <- Jumps around
          └─────────────────────────────────────────────────┘
          0                    10M elements

Each [####] = 8192 elements = separate L3 cache region accessed
Result: Cores evict each other's data from shared L3 cache
```

### The Fix

```cpp
// constants.hpp (new)
constexpr int MIN_PARALLEL_SORT_SIZE = 65536;  // 256KB per task
```

With 65536 threshold:
- Number of tasks: 10,000,000 / 65,536 ≈ **153 tasks**
- Less task stealing → better cache locality
- Each thread works on larger contiguous regions

```
Memory Access Pattern with Large Tasks:

Thread 0: [################]                                  <- Contiguous
Thread 1:                   [################]                <- Contiguous
Thread 2:                                      [##############] <- Contiguous
          └─────────────────────────────────────────────────┘
          0                    10M elements

Each [################] = 65536 elements = fits in L2 cache
Result: Each core's working set stays in private L2 cache
```

### Why 65536?

| Value | Size | Tasks (10M) | Fits in L2? | Analysis |
|-------|------|-------------|-------------|----------|
| 8192 | 32KB | 1220 | Yes | Too many tasks, cache thrashing |
| 32768 | 128KB | 305 | Yes | Moderate |
| **65536** | **256KB** | **153** | **Yes** | **Sweet spot** |
| 131072 | 512KB | 76 | No | Spills L2 |

L2 cache on Raptor Lake P-core = 1.25MB. We want partition to fit comfortably with room for pivots and temp storage.

### File to Modify

```
include/dpqs/constants.hpp
Line 12: constexpr int MIN_PARALLEL_SORT_SIZE = 8192;
      → constexpr int MIN_PARALLEL_SORT_SIZE = 65536;
```

---

## Solution 5: Cache Line Padding

### Current Problem

```cpp
// threadpool.hpp (current)
struct WorkStealingQueue {
    std::deque<std::function<void()>> q;  // ~32 bytes
    std::mutex mtx;                        // ~40 bytes
    // Total: ~72 bytes - spans 2 cache lines!
};

std::vector<std::unique_ptr<WorkStealingQueue>> queues;  // Adjacent in memory
```

### What is False Sharing?

CPU cache operates in 64-byte units called "cache lines":

```
Physical Memory Layout (BEFORE padding):
┌────────────────────────────────────────────────────────────────┐
│                        Cache Line 0 (64 bytes)                  │
│ ┌──────────────────────────────┬───────────────────────────────┤
│ │     WorkStealingQueue[0]      │   WorkStealingQueue[1]        │
│ │     (Thread 0 modifies)       │   (Thread 1 modifies)         │
│ └──────────────────────────────┴───────────────────────────────┘
│                    ↑ SHARED CACHE LINE ↑                        │
└────────────────────────────────────────────────────────────────┘
```

When Thread 0 and Thread 1 modify their respective queues:

```
Time T0: Thread 0 locks Queue[0].mtx
         → Core 0's cache line marked "Modified"
         → Core 1's copy INVALIDATED

Time T1: Thread 1 locks Queue[1].mtx
         → Core 1 must reload from L3
         → Core 1's cache line marked "Modified"
         → Core 0's copy INVALIDATED

Time T2: Thread 0 unlocks Queue[0].mtx
         → Core 0 must reload from L3
         → Cache line bounces back...

Result: "Ping-pong" between cores via L3 cache
```

### The Fix

```cpp
// threadpool.hpp (new)
struct alignas(64) WorkStealingQueue {  // Force 64-byte alignment
    std::deque<std::function<void()>> q;
    std::mutex mtx;
    // Compiler automatically pads to 64-byte boundary
};
```

```
Physical Memory Layout (AFTER padding):
┌────────────────────────────────────────────────────────────────┐
│   Cache Line 0 (64 bytes)     │   Cache Line 1 (64 bytes)      │
│ ┌────────────────────────────┐│┌────────────────────────────┐  │
│ │   WorkStealingQueue[0]     │││   WorkStealingQueue[1]     │  │
│ │   (Thread 0 EXCLUSIVE)     │││   (Thread 1 EXCLUSIVE)     │  │
│ └────────────────────────────┘│└────────────────────────────┘  │
│         ↑ SEPARATE CACHE LINES - NO INTERFERENCE ↑             │
└────────────────────────────────────────────────────────────────┘
```

### Additional Padding for Atomics

```cpp
// threadpool.hpp - Also pad frequently-written atomics
alignas(64) std::atomic<bool> stop{false};
alignas(64) std::atomic<long> incomplete_tasks{0};
```

### Files to Modify

```
include/dpqs/parallel/threadpool.hpp
Line 33: struct WorkStealingQueue {
      → struct alignas(64) WorkStealingQueue {

Line 59: std::atomic<bool> stop{false};
      → alignas(64) std::atomic<bool> stop{false};

Line 60: std::atomic<long> incomplete_tasks{0};
      → alignas(64) std::atomic<long> incomplete_tasks{0};
```

---

## Solution 6: Prefetching

### Current Problem

During partitioning, CPU fetches data one cache line at a time as needed:

```cpp
// Current partition loop
while (k <= gt) {
    T val = a[k];  // CPU requests a[k], waits ~100 cycles for L3/DRAM
    // ... process val ...
    k++;
}
```

### The Fix

Tell CPU to start loading future data while processing current data:

```cpp
// With prefetching
while (k <= gt) {
    // Start loading data we'll need 128 elements from now
    __builtin_prefetch(&a[k + 128], 0, 3);  // 0=read, 3=keep in all cache levels

    T val = a[k];  // This data should already be in cache!
    // ... process val ...
    k++;
}
```

### Prefetch Intrinsic Parameters

```cpp
__builtin_prefetch(address, rw, locality);
//                          │    │
//                          │    └── 0=no temporal locality (use once)
//                          │        1=low temporal locality
//                          │        2=medium temporal locality
//                          │        3=high temporal locality (keep in all caches)
//                          │
//                          └── 0=read, 1=write
```

For sorting, we use `(0, 3)` because:
- We're reading elements for comparison
- We access elements multiple times during partitioning (high locality)

### Where to Add Prefetch

```cpp
// partition.hpp - In partition_dual_pivot
while (k <= gt) {
    __builtin_prefetch(&a[k + 128], 0, 3);  // ADD THIS LINE

    if (comp(a[k], pivot1)) {
        // ...
    }
}
```

### Files to Modify

```
include/dpqs/partition.hpp
Inside partition_dual_pivot function, at start of while loop
```

---

## Step 1: Validation Plan

### Before Implementation

```bash
# Baseline measurements (current code)
cd benchmarks
g++ -std=c++20 -O2 -g -pthread -I../include vtune_profile.cpp -o vtune_profile.exe

# Run baseline for all thread counts
./vtune_profile.exe 1   # Record: ___ms
./vtune_profile.exe 4   # Record: ___ms
./vtune_profile.exe 8   # Record: ___ms
./vtune_profile.exe 16  # Record: ___ms

# VTune baseline (16 threads)
vtune -collect uarch-exploration -result-dir step1_baseline -- ./vtune_profile.exe 16
vtune -report summary -result-dir step1_baseline
# Record: L3 Bound %, Memory Bound %
```

### After Implementation

```bash
# Recompile
g++ -std=c++20 -O2 -g -pthread -I../include vtune_profile.cpp -o vtune_profile.exe

# Run with changes
./vtune_profile.exe 1   # Compare to baseline
./vtune_profile.exe 4   # Compare to baseline
./vtune_profile.exe 8   # Compare to baseline
./vtune_profile.exe 16  # Compare to baseline

# VTune after changes (16 threads)
vtune -collect uarch-exploration -result-dir step1_after -- ./vtune_profile.exe 16
vtune -report summary -result-dir step1_after
# Compare: L3 Bound % should decrease significantly
```

### Success Criteria

| Metric | Baseline | Target | Pass? |
|--------|----------|--------|-------|
| 16-thread time | 98ms | <85ms | |
| L3 Bound % | 37.9% | <20% | |
| Memory Bound % | 41.1% | <25% | |
| 1-thread time | 508ms | No regression | |

### Rollback Plan

If performance decreases:
```bash
git checkout include/dpqs/constants.hpp
git checkout include/dpqs/parallel/threadpool.hpp
git checkout include/dpqs/partition.hpp
```

---

# Step 2: Block-Based Partitioning

## Overview

| Aspect | Value |
|--------|-------|
| Risk | Medium |
| Effort | High (100+ lines) |
| Target | Branch misprediction (35% → <10%) |
| Expected Gain | 15-25% at 1-4 threads |

## Current Problem

```cpp
// Current element-by-element partitioning
while (k <= gt) {
    if (comp(a[k], pivot1)) {           // BRANCH - 35% misprediction
        swap(a[k], a[lt++]);
    } else if (comp(pivot2, a[k])) {    // BRANCH - more misprediction
        // ...
    }
    k++;
}
```

CPU branch predictor struggles because:
- Random data → comparisons unpredictable
- Each element's destination depends on value vs. pivots
- ~35% of branches are mispredicted (VTune measured)
- Each misprediction costs ~15-20 cycles (pipeline flush)

## Technical Solution: Two-Phase Partitioning

### Phase 1: Classification (Branchless)

Process 64 elements at once, storing only the classification:

```cpp
constexpr size_t BLOCK_SIZE = 64;

// Classification buffer
alignas(64) uint8_t classification[BLOCK_SIZE];  // 0=left, 1=middle, 2=right
size_t left_count = 0, middle_count = 0, right_count = 0;

// Classify all 64 elements WITHOUT moving them
for (size_t i = 0; i < BLOCK_SIZE; i++) {
    T val = a[block_start + i];

    // Branchless classification using arithmetic
    bool goes_left = comp(val, pivot1);
    bool goes_right = comp(pivot2, val);

    // This compiles to conditional moves (cmov), NOT branches
    uint8_t dest = goes_left ? 0 : (goes_right ? 2 : 1);
    classification[i] = dest;

    // Branchless counting using addition
    left_count += goes_left;
    right_count += goes_right;
}
middle_count = BLOCK_SIZE - left_count - right_count;
```

### Why is Classification Branchless?

The ternary operator with simple values compiles to `cmov`:

```asm
; Compiler output for: dest = goes_left ? 0 : (goes_right ? 2 : 1)
cmp     val, pivot1          ; Compare
setl    al                   ; al = (val < pivot1)
cmp     val, pivot2          ; Compare
setg    bl                   ; bl = (val > pivot2)
; ... arithmetic to combine into dest
; NO BRANCH INSTRUCTIONS
```

### Phase 2: Rearrangement (Predictable Branches)

Now we know exactly how many elements go where:

```cpp
// We know: left_count elements go left, right_count go right
// These loops have PERFECTLY PREDICTABLE branches

// Output buffers (or in-place with careful indexing)
size_t left_idx = 0, right_idx = 0;

for (size_t i = 0; i < BLOCK_SIZE; i++) {
    // Branch is predictable: we process ALL lefts, then ALL middles, etc.
    switch (classification[i]) {
        case 0: // Left
            swap(a[block_start + i], a[lt + left_idx]);
            left_idx++;
            break;
        case 2: // Right
            swap(a[block_start + i], a[gt - right_idx]);
            right_idx++;
            break;
        // Middle elements stay in place
    }
}

lt += left_count;
gt -= right_count;
```

### Full Block Partition Function

```cpp
template<typename T, typename Compare>
std::pair<std::ptrdiff_t, std::ptrdiff_t> partition_dual_pivot_block(
    T* a, std::ptrdiff_t low, std::ptrdiff_t high,
    std::ptrdiff_t pivotIndex1, std::ptrdiff_t pivotIndex2, Compare comp)
{
    constexpr size_t BLOCK_SIZE = 64;

    using std::swap;
    swap(a[low], a[pivotIndex1]);
    swap(a[high - 1], a[pivotIndex2]);

    T pivot1 = a[low];
    T pivot2 = a[high - 1];

    std::ptrdiff_t lt = low + 1;
    std::ptrdiff_t gt = high - 2;
    std::ptrdiff_t k = lt;

    alignas(64) uint8_t left_block[BLOCK_SIZE];
    alignas(64) uint8_t right_block[BLOCK_SIZE];

    // Process full blocks
    while (k + BLOCK_SIZE <= gt) {
        size_t left_count = 0;
        size_t right_count = 0;

        // Phase 1: Classify
        for (size_t i = 0; i < BLOCK_SIZE; i++) {
            bool goes_left = comp(a[k + i], pivot1);
            bool goes_right = comp(pivot2, a[k + i]);

            // Store indices, not classifications
            left_block[left_count] = i;
            left_count += goes_left;

            right_block[right_count] = i;
            right_count += goes_right;
        }

        // Phase 2: Rearrange
        // Move elements < pivot1 to left partition
        for (size_t i = 0; i < left_count; i++) {
            swap(a[k + left_block[i]], a[lt++]);
        }

        // Move elements > pivot2 to right partition
        for (size_t i = 0; i < right_count; i++) {
            swap(a[k + right_block[i]], a[gt--]);
        }

        k += BLOCK_SIZE;
    }

    // Handle remaining elements with original algorithm
    while (k <= gt) {
        if (comp(a[k], pivot1)) {
            swap(a[k], a[lt++]);
            k++;
        } else if (comp(pivot2, a[k])) {
            while (k < gt && comp(pivot2, a[gt])) gt--;
            swap(a[k], a[gt--]);
            if (comp(a[k], pivot1)) {
                swap(a[k], a[lt++]);
            }
            k++;
        } else {
            k++;
        }
    }

    --lt;
    ++gt;
    swap(a[low], a[lt]);
    swap(a[high - 1], a[gt]);

    return {lt, gt};
}
```

### Branch Count Comparison

| Approach | Branches per N elements | Mispredictions (35% rate) |
|----------|------------------------|---------------------------|
| Element-by-element | 2N | 0.70N |
| Block-based (64) | N/64 + N | 0.35N/64 + ~0 ≈ 0.005N |

Block-based achieves **~140x fewer mispredictions**.

## Files to Modify

```
include/dpqs/partition.hpp
- Add new function: partition_dual_pivot_block
- Modify callers to use block version for large partitions

include/dpqs/parallel/parallel_sort.hpp
- Call partition_dual_pivot_block when size > 256
- Fall back to original for small partitions
```

## Step 2: Validation Plan

### Success Criteria

| Metric | After Step 1 | Target | Pass? |
|--------|--------------|--------|-------|
| 1-thread time | ~500ms | <420ms | |
| 4-thread time | ~154ms | <130ms | |
| Branch Mispredict % | 35% | <15% | |
| 16-thread time | Step 1 result | No regression | |

---

# Step 3: Lock-Free Work Stealing (OPTIONAL)

## When to Do This Step

Only proceed if after Steps 1 and 2:
- VTune still shows >20% time in synchronization
- `sched_yield` or `pthread_mutex` appears in hotspots
- Performance at 16 threads plateaus

## Current Problem

```cpp
// Current threadpool.hpp
struct WorkStealingQueue {
    std::mutex mtx;  // Every operation locks this!

    void push(task) {
        std::lock_guard<std::mutex> lock(mtx);  // LOCK
        q.push_back(task);
    }

    bool try_steal(task) {
        std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);  // TRY LOCK
        if (!lock) return false;  // Failed - mutex busy
        // ...
    }
};
```

VTune showed:
- 37.8% time in `sched_yield` (thread yielding while waiting)
- 7.3% time in `pthread_mutex_trylock`

## Technical Solution: Chase-Lev Deque

### Key Insight

In work-stealing, there are two types of operations:
1. **Owner operations** (push/pop): Only the owning thread performs these
2. **Thief operations** (steal): Any other thread can attempt

These access DIFFERENT ends of the queue → can be made lock-free!

```
Chase-Lev Deque Structure:

      THIEVES                              OWNER
       steal                            push/pop
         ↓                                  ↓
   ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┐
   │  T  │  T  │  T  │  T  │  T  │  T  │  T  │  ← Circular buffer
   └─────┴─────┴─────┴─────┴─────┴─────┴─────┘
      ↑                                    ↑
     top                                bottom
   (atomic)                            (atomic)
```

### Memory Ordering Requirements

```cpp
// The tricky part: correct memory ordering

// Owner push - no contention with other owners
void push(Task* task) {
    int64_t b = bottom.load(std::memory_order_relaxed);
    buffer[b % SIZE].store(task, std::memory_order_relaxed);

    // CRITICAL: Ensure task is visible before incrementing bottom
    std::atomic_thread_fence(std::memory_order_release);

    bottom.store(b + 1, std::memory_order_relaxed);
}

// Owner pop - may race with thieves
Task* pop() {
    int64_t b = bottom.load(std::memory_order_relaxed) - 1;
    bottom.store(b, std::memory_order_relaxed);

    // CRITICAL: Ensure bottom decrement is visible to thieves
    std::atomic_thread_fence(std::memory_order_seq_cst);

    int64_t t = top.load(std::memory_order_relaxed);

    if (t <= b) {
        // Queue not empty
        Task* task = buffer[b % SIZE].load(std::memory_order_relaxed);

        if (t == b) {
            // Last element - race with thieves!
            if (!top.compare_exchange_strong(
                    t, t + 1,
                    std::memory_order_seq_cst,
                    std::memory_order_relaxed)) {
                // Lost race to thief
                bottom.store(b + 1, std::memory_order_relaxed);
                return nullptr;
            }
            bottom.store(b + 1, std::memory_order_relaxed);
        }
        return task;
    }

    // Queue was empty
    bottom.store(b + 1, std::memory_order_relaxed);
    return nullptr;
}

// Thief steal - may race with owner and other thieves
Task* steal() {
    int64_t t = top.load(std::memory_order_acquire);

    // CRITICAL: Ensure we see consistent view
    std::atomic_thread_fence(std::memory_order_seq_cst);

    int64_t b = bottom.load(std::memory_order_acquire);

    if (t < b) {
        // Queue not empty
        Task* task = buffer[t % SIZE].load(std::memory_order_relaxed);

        // Try to claim this task
        if (top.compare_exchange_strong(
                t, t + 1,
                std::memory_order_seq_cst,
                std::memory_order_relaxed)) {
            return task;  // Success!
        }
        // CAS failed - another thief won, return nullptr
    }
    return nullptr;
}
```

### Why Lock-Free is Faster

| Operation | Mutex-based | Lock-free |
|-----------|-------------|-----------|
| Owner push | ~50 cycles (lock+unlock) | ~5 cycles (atomic store) |
| Owner pop | ~50 cycles | ~10 cycles (usually no CAS) |
| Thief steal (success) | ~100 cycles (try_lock + ops) | ~20 cycles (single CAS) |
| Thief steal (contention) | ~500+ cycles (spin, yield) | ~10 cycles (CAS fail, move on) |

### Complete Implementation

```cpp
template<size_t CAPACITY = 4096>
class ChaseLevDeque {
private:
    alignas(64) std::atomic<int64_t> top_{0};
    alignas(64) std::atomic<int64_t> bottom_{0};
    alignas(64) std::array<std::atomic<std::function<void()>*>, CAPACITY> buffer_;

public:
    ChaseLevDeque() {
        for (auto& slot : buffer_) {
            slot.store(nullptr, std::memory_order_relaxed);
        }
    }

    void push(std::function<void()>* task) {
        int64_t b = bottom_.load(std::memory_order_relaxed);
        buffer_[b % CAPACITY].store(task, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_release);
        bottom_.store(b + 1, std::memory_order_relaxed);
    }

    std::function<void()>* pop() {
        int64_t b = bottom_.load(std::memory_order_relaxed) - 1;
        bottom_.store(b, std::memory_order_relaxed);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        int64_t t = top_.load(std::memory_order_relaxed);

        if (t <= b) {
            auto* task = buffer_[b % CAPACITY].load(std::memory_order_relaxed);
            if (t == b) {
                if (!top_.compare_exchange_strong(t, t + 1,
                        std::memory_order_seq_cst, std::memory_order_relaxed)) {
                    bottom_.store(b + 1, std::memory_order_relaxed);
                    return nullptr;
                }
                bottom_.store(b + 1, std::memory_order_relaxed);
            }
            return task;
        }

        bottom_.store(b + 1, std::memory_order_relaxed);
        return nullptr;
    }

    std::function<void()>* steal() {
        int64_t t = top_.load(std::memory_order_acquire);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        int64_t b = bottom_.load(std::memory_order_acquire);

        if (t < b) {
            auto* task = buffer_[t % CAPACITY].load(std::memory_order_relaxed);
            if (top_.compare_exchange_strong(t, t + 1,
                    std::memory_order_seq_cst, std::memory_order_relaxed)) {
                return task;
            }
        }
        return nullptr;
    }

    bool empty() {
        int64_t t = top_.load(std::memory_order_relaxed);
        int64_t b = bottom_.load(std::memory_order_relaxed);
        return t >= b;
    }
};
```

## Files to Modify

```
include/dpqs/parallel/threadpool.hpp
- Replace WorkStealingQueue struct with ChaseLevDeque class
- Update all queue operations in ThreadPool class
- Handle task memory management (new/delete for function pointers)
```

## Step 3: Validation Plan

### Success Criteria

| Metric | After Step 2 | Target | Pass? |
|--------|--------------|--------|-------|
| 16-thread time | ~70ms | <55ms | |
| sched_yield % | >20% | <5% | |
| Spin Time % | >30% | <10% | |

---

# Summary: Expected Results

## Performance Projection

| Threads | Baseline | After Step 1 | After Step 2 | After Step 3 |
|---------|----------|--------------|--------------|--------------|
| 1 | 508ms | 508ms | **~420ms** | ~420ms |
| 4 | 154ms | ~145ms | **~120ms** | ~115ms |
| 8 | 110ms | ~95ms | ~85ms | **~75ms** |
| 16 | 98ms | **~80ms** | ~70ms | **~55ms** |

## Risk Assessment

| Step | Risk | Rollback Difficulty |
|------|------|---------------------|
| Step 1 | Low | Easy (3 files, small changes) |
| Step 2 | Medium | Moderate (can keep both functions) |
| Step 3 | High | Hard (significant rewrite) |

## Recommendation

1. **Definitely do Step 1** - low risk, measurable gains
2. **Do Step 2 if** - you need single-threaded performance
3. **Do Step 3 only if** - Steps 1+2 don't reduce sync overhead

---

# Appendix: Quick Reference

## Files Changed Per Step

### Step 1
- `include/dpqs/constants.hpp` (1 line)
- `include/dpqs/parallel/threadpool.hpp` (3 lines)
- `include/dpqs/partition.hpp` (2 lines)

### Step 2
- `include/dpqs/partition.hpp` (~100 lines new function)
- `include/dpqs/parallel/parallel_sort.hpp` (modify partition call)

### Step 3
- `include/dpqs/parallel/threadpool.hpp` (rewrite ~200 lines)

## Compile Command

```bash
cd benchmarks
g++ -std=c++20 -O2 -g -pthread -I../include vtune_profile.cpp -o vtune_profile.exe
```

## VTune Commands

```bash
# Quick timing
./vtune_profile.exe 16

# Full analysis
vtune -collect uarch-exploration -result-dir results_stepN -- ./vtune_profile.exe 16
vtune -report summary -result-dir results_stepN
```
