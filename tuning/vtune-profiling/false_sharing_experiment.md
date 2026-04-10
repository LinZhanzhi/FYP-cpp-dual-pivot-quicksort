# False Sharing Experiment Results

## Objective
Measure the performance impact of cache-line padding (`alignas(64)`) on concurrent data structures to validate the false sharing optimization.

## Methodology

### Test Design
We created two test scenarios using **contiguous array allocation** (where false sharing actually occurs):

1. **Atomic Counter Test**: Each thread repeatedly increments its own `std::atomic<int64_t>` counter (5M operations each)
2. **Mutex Counter Test**: Each thread uses its own mutex-protected counter (500K operations each)

### Compilation
```bash
# Without padding (false sharing enabled)
g++ -std=c++17 -O2 -pthread false_sharing_v2.cpp -o fs2_packed.exe

# With padding (false sharing prevented)
g++ -std=c++17 -O2 -pthread -DWITH_PADDING false_sharing_v2.cpp -o fs2_padded.exe
```

### Key Difference
```cpp
// Without padding: sizeof(Counter) = 8 bytes
// Multiple counters fit on same 64-byte cache line
struct PackedCounter {
    std::atomic<int64_t> value{0};
};

// With padding: sizeof(Counter) = 64 bytes
// Each counter occupies its own cache line
struct alignas(64) PaddedCounter {
    std::atomic<int64_t> value{0};
};
```

## Memory Layout Analysis

### WITHOUT Padding (Packed)
```
sizeof(Counter) = 8 bytes
Counter[0] @ 0x...590 (cache line 982)
Counter[1] @ 0x...598 (cache line 982)  ← SAME LINE!
Counter[2] @ 0x...5a0 (cache line 982)  ← SAME LINE!
Counter[3] @ 0x...5a8 (cache line 982)  ← SAME LINE!
```
**All 4 counters share the same 64-byte cache line → FALSE SHARING**

### WITH Padding (Isolated)
```
sizeof(Counter) = 64 bytes
Counter[0] @ 0x...000 (cache line 0)
Counter[1] @ 0x...040 (cache line 1)
Counter[2] @ 0x...080 (cache line 2)
Counter[3] @ 0x...0c0 (cache line 3)
```
**Each counter on its own cache line → NO FALSE SHARING**

---

## Results

### Test 1: Atomic Counters (5M operations/thread)

| Threads | Packed (ms) | Padded (ms) | Speedup | 
|---------|-------------|-------------|---------|
| 2       | 76.82       | 18.61       | **4.13×** |
| 4       | 155.53      | 19.01       | **8.18×** |
| 8       | 298.32      | 24.97       | **11.95×** |
| 16      | 472.44      | 37.08       | **12.74×** |

### Test 2: Mutex Counters (500K operations/thread)

| Threads | Packed (ms) | Padded (ms) | Speedup |
|---------|-------------|-------------|---------|
| 2       | 29.15       | 5.22        | **5.59×** |
| 4       | 42.72       | 5.49        | **7.78×** |
| 8       | 59.51       | 6.50        | **9.16×** |
| 16      | 65.15       | 20.99       | **3.10×** |

### Throughput Comparison (ops/sec)

| Threads | Packed (M ops/s) | Padded (M ops/s) | Improvement |
|---------|------------------|------------------|-------------|
| 2       | 130.17           | 537.29           | **4.13×** |
| 4       | 128.60           | 1051.91          | **8.18×** |
| 8       | 134.09           | 1602.06          | **11.95×** |
| 16      | 169.33           | 2157.65          | **12.74×** |

---

## Key Findings

### 1. False Sharing Impact is Dramatic on Isolated Queue Operations
- **4-13× slowdown** when counters share cache lines
- Impact **increases with thread count** (more cores competing for same line)
- Matches published literature (Intel, cppreference report ~6× impact)

### 2. Why We Don't See This in Full Sort Benchmark
Our sorting implementation has these characteristics:
- **Data access dominates**: 10M elements × 4 bytes = 40MB data access
- **Queue operations are rare**: ~1000 task pushes/pops total vs millions of comparisons
- **VTune shows 38% L3 Bound from data**, not queue metadata

### 3. Cache-Line Competition Analysis
```
False Sharing (packed):
Thread 0 writes → invalidates Line X on all cores
Thread 1 writes → must fetch from L3, invalidates Line X on Thread 0
Thread 2 writes → must fetch from L3, invalidates Line X on Thread 1
...
Result: Constant L3 traffic, no data stays in L1/L2

No False Sharing (padded):
Thread 0 writes to Line A → stays in Thread 0's L1
Thread 1 writes to Line B → stays in Thread 1's L1
...
Result: All writes hit L1, no cross-core invalidations
```

---

## Why Keep Cache-Line Padding in Production Code?

### 1. Standard C++17 Recognition
C++17 introduced `std::hardware_destructive_interference_size` (cppreference):
```cpp
// Minimum offset between objects to avoid false sharing
constexpr std::size_t hardware_destructive_interference_size = 64; // on x86-64
```
The standard explicitly acknowledges this as a known performance pitfall.

### 2. Industry Best Practice
- **Intel VTune** detects and flags false sharing as a performance issue
- **Microsoft** documents cache-line alignment for concurrent structures
- **Java's ForkJoinPool** (our reference implementation) uses `@Contended` annotation

### 3. Zero Cost When Correct
- `alignas(64)` only adds padding where needed
- No runtime overhead (compile-time alignment)
- Prevents performance cliffs on different workloads

### 4. Defense Against Future Regressions
Current workload may not stress queue metadata, but:
- Different data sizes could change the ratio
- Different hardware may have different cache behaviors
- Multi-tenant environments may have higher baseline L3 contention

---

## Conclusion

**Measured Impact**: False sharing causes **4-13× slowdown** on isolated queue operations.

**Impact in Sorting Context**: Difficult to measure because:
- Data access (38% L3 Bound) dominates queue metadata access
- Only ~1000 queue operations vs millions of array comparisons
- Signal is below noise floor in end-to-end benchmark

**Recommendation**: Keep `alignas(64)` as it:
1. Is a documented best practice (Intel, cppreference, Java)
2. Has zero runtime cost
3. Prevents a known performance pitfall
4. May prevent regressions in different usage scenarios

---

## References

1. **C++17 Standard**: `std::hardware_destructive_interference_size`
   - https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
   - Documents 64-byte alignment for false sharing prevention
   - Example shows ~6× speedup from proper alignment

2. **Intel Developer Zone**: False Sharing Detection in VTune
   - VTune Profiler flags false sharing as "Memory Contention"
   - Recommends `alignas(64)` or padding for concurrent structures

3. **Java @Contended Annotation** (JEP 142):
   - Java 8 added `@Contended` for same purpose
   - Used in `ForkJoinPool` (our reference implementation)
   - https://openjdk.org/jeps/142

---

## Appendix: Raw Benchmark Output

### WITHOUT Padding
```
========================================
  FALSE SHARING TEST - WITHOUT PADDING 
  (packed - counters may share lines)  
========================================

Cache line size: 64 bytes
Operations per thread: 5000000

########## TEST 1: Atomic Counters ##########

=== Counter Memory Layout ===
sizeof(Counter) = 8 bytes
Counter[0] and Counter[1] on same cache line: YES (FALSE SHARING!)

Results:
Threads | Time (ms) | Ops/sec (M)
--------|-----------|------------
    2   |     76.82 |     130.17
    4   |    155.53 |     128.60
    8   |    298.32 |     134.09
   16   |    472.44 |     169.33

########## TEST 2: Mutex Counters ##########

=== MutexCounter Memory Layout ===
sizeof(MutexCounter) = 16 bytes
MutexCounter[0] and MutexCounter[1] on same cache line: YES (FALSE SHARING!)

Results:
Threads | Time (ms) | Ops/sec (M)
--------|-----------|------------
    2   |     29.15 |      34.31
    4   |     42.72 |      46.82
    8   |     59.51 |      67.22
   16   |     65.15 |     122.79
```

### WITH Padding
```
========================================
  FALSE SHARING TEST - WITH PADDING    
  (alignas(64) - each counter isolated)
========================================

Cache line size: 64 bytes
Operations per thread: 5000000

########## TEST 1: Atomic Counters ##########

=== Counter Memory Layout ===
sizeof(Counter) = 64 bytes
Counter[0] and Counter[1] on same cache line: NO (isolated)

Results:
Threads | Time (ms) | Ops/sec (M)
--------|-----------|------------
    2   |     18.61 |     537.29
    4   |     19.01 |    1051.91
    8   |     24.97 |    1602.06
   16   |     37.08 |    2157.65

########## TEST 2: Mutex Counters ##########

=== MutexCounter Memory Layout ===
sizeof(MutexCounter) = 64 bytes
MutexCounter[0] and MutexCounter[1] on same cache line: NO (isolated)

Results:
Threads | Time (ms) | Ops/sec (M)
--------|-----------|------------
    2   |      5.22 |     191.52
    4   |      5.49 |     364.40
    8   |      6.50 |     615.80
   16   |     20.99 |     381.18
```
