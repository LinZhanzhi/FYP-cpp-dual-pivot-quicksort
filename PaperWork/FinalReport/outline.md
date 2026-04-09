# Final Report Outline
## Dual-Pivot Quicksort: A High-Performance C++ Implementation

> **Guiding Principle — Story-Based Structure**
> Each optimization is told as ONE complete narrative: problem → design → implementation → tuning → result.
> Readers follow the full story without jumping between chapters.

---

## Preamble (Not counted in 50-page limit)

### 1. Cover Page
- Title: "Dual-Pivot Quicksort: A High-Performance C++ Implementation"
- Student ID
- Programme Stream
- Supervisor Name
- Date: April 2026

### 2. Abstract (~250 words)
- Problem: Implementing and optimizing Yaroslavskiy's dual-pivot quicksort in C++
- Approach: Sequential implementation with adaptive optimizations + parallel work-stealing architecture
- Key Results: Up to 19x speedup on structured data vs std::sort; 5.18x parallel speedup (16 threads)
- Contribution: Header-only library, empirical tuning methodology, hardware limitation analysis

### 3. Table of Contents

### 4. List of Tables and Figures

---

## Main Body (~45-50 pages)

### Chapter 1: Introduction (4-5 pages)
#### 1.1 Background and Motivation
- Sorting as a fundamental operation
- Java 7's adoption of dual-pivot quicksort (Yaroslavskiy, 2009)
- Gap: No production-quality C++ implementation exists

#### 1.2 Problem Statement
- Adapt and optimize dual-pivot quicksort for modern C++ (C++17/20)
- Investigate parallel scaling characteristics
- Provide STL-compatible, header-only library

#### 1.3 Objectives
1. Implement faithful adaptation of Yaroslavskiy's algorithm
2. Optimize for modern hardware (cache, SIMD-friendly layout)
3. Develop parallel version using work-stealing
4. Benchmark against std::sort across diverse data patterns
5. Document algorithm engineering decisions

#### 1.4 Scope and Limitations
- In scope: Comparison-based sorting, primitive types, parallel execution
- Out of scope: Stable sorting, GPU acceleration, distributed systems

#### 1.5 Related Work: Positioning This Project
This section provides a brief overview of the algorithmic landscape. Detailed connections to prior work appear in each chapter as part of the problem-solving narrative.

**Sorting Algorithm Landscape**:
| Algorithm | Complexity | Key Feature | Weakness |
|-----------|------------|-------------|----------|
| Hoare's Quicksort (1962) | O(n log n) avg | Simple, in-place | O(n²) worst case |
| Introsort (std::sort) | O(n log n) guaranteed | Hybrid (QS + heapsort) | No structure detection |
| Timsort (Python, Java) | O(n log n) | Run merging | O(n) space always |
| pdqsort | O(n log n) | Pattern-defeating | Single pivot only |
| **Dual-Pivot QS** | O(n log n) | Two pivots + runs | O(n) space for runs |

**Key Research Foundations** (cited in context throughout):
- Yaroslavskiy (2009): Dual-pivot partitioning
- Wild (2012): Mathematical analysis of dual-pivot efficiency
- Blumofe & Leiserson (1999): Work-stealing paradigm
- Peters (2021): Pattern-defeating quicksort

#### 1.6 Report Organization
- **Chapter 2**: Core dual-pivot quicksort algorithm
- **Chapter 3**: Adaptive optimizations (run merging, type-specific paths)
- **Chapter 4**: Parallel execution and hardware limits
- **Chapter 5**: Results and evaluation
- **Chapter 6**: Discussion
- **Chapter 7**: Conclusion

---

### Chapter 2: Core Algorithm — Dual-Pivot Quicksort (8-10 pages)

This chapter presents the complete story of the core dual-pivot quicksort algorithm: from the foundational theory, through implementation details, to empirical tuning of key parameters.

#### 2.1 Prior Work: From Hoare to Yaroslavskiy

**Classical Quicksort** (Hoare, 1962):
- Single pivot partitions array into two regions: [≤ pivot] [> pivot]
- Expected O(n log n) comparisons, O(n²) worst case
- Dominant sorting algorithm for 50+ years due to cache efficiency

**The Dual-Pivot Innovation** (Yaroslavskiy, 2009):
- Counterintuitive insight: Using TWO pivots is faster than one
- Three-way partitioning: [< P1] [P1 ≤ x ≤ P2] [> P2]
- Adopted by Java 7 (2011), now default for primitive arrays

**Why Is It Faster?** (Wild, 2012):
Wild's doctoral thesis provided the mathematical explanation:
- Single-pivot: ~2n comparisons, ~0.67n swaps per partition
- Dual-pivot: ~1.9n comparisons, ~0.6n swaps per partition
- **Key insight**: Fewer element SCANS despite more comparisons
- Cache effect: Scanning 3 smaller regions beats 2 larger regions

#### 2.2 The Algorithm
- Core invariant: [< P1] [P1 ≤ x ≤ P2] [> P2]
- Pivot selection: Median-of-5 at equidistant positions
- Special case: When P1 == P2, fall back to Dutch National Flag

#### 2.3 Partitioning Implementation (partition.hpp)
**The Problem**: Efficiently divide array into three regions around two pivots.

**Design**: 
- Three-way partitioning using three pointers: `lt` (left), `k` (scanner), `gt` (right)
- `k` advances forward through unprocessed elements
- `gt` scans backward when finding elements > P2, creating short-range sequential access

**Why This Is Cache-Friendly**:
When an element belongs in the rightmost partition (> P2), the algorithm scans `gt` backward. This backward scan:
1. Accesses memory in a **sequential pattern** (reverse order, but still predictable)
2. Benefits from **hardware prefetcher** which tracks both forward and reverse strides
3. Keeps accesses within a **small memory region** (the unprocessed portion)
4. Works well with **`__builtin_prefetch`** hints for upcoming elements

**Implementation**:
- Main loop classifies elements into three regions
- Prefetch 64 elements ahead: `__builtin_prefetch(&a[k + 64], 0, 3)`
- Dutch National Flag fallback for single-pivot (many duplicates)
- Prevents O(n²) degradation on all-equal arrays

#### 2.4 Pivot Selection (sequential_sorters.hpp)
**The Problem**: Poor pivot selection causes O(n²) worst case. When the pivot consistently lands near array extremes, partitions become highly unbalanced.

**Prior Work on Pivot Selection**:
| Strategy | Source | Expected Comparisons |
|----------|--------|---------------------|
| Random/first element | Hoare (1962) | 1.386 n log n |
| Median-of-3 | Sedgewick (1978) | 1.188 n log n |
| Pseudomedian-of-9 | Bentley & McIlroy (1993) | Further improved |

**Why Median-of-5?**
- Sedgewick's median-of-3 (first, middle, last) is vulnerable to adversarial patterns
- Bentley & McIlroy's "ninther" (median of three medians-of-3) requires 12 comparisons
- **Median-of-5 is a practical middle ground**: Better robustness than median-of-3, less overhead than ninther

**Design**:
- Sample 5 elements at **equidistant interior positions** (not at edges)
- Positions: e1 ≈ 3/8, e2 ≈ 7/16, e3 ≈ 1/2, e4 ≈ 9/16, e5 ≈ 5/8
- **Why avoid edges?** Pre-sorted or reverse-sorted data has extremes at boundaries — sampling interior positions avoids selecting them as pivots

**Implementation**:
```cpp
std::ptrdiff_t step = (size >> 3) * 3 + 3;  // ≈ 3/8 of size
std::ptrdiff_t e1 = low + step;              // 3/8 from start
std::ptrdiff_t e5 = end - step;              // 3/8 from end
std::ptrdiff_t e3 = (e1 + e5) >> 1;          // Middle
std::ptrdiff_t e2 = (e1 + e3) >> 1;          // Between e1 and e3
std::ptrdiff_t e4 = (e3 + e5) >> 1;          // Between e3 and e5
```

**Sorting Network**: Optimal 9-comparator network for 5 elements (Bose-Nelson, 1962)
- Sorts the 5 samples in exactly 9 comparisons (theoretical minimum)
- After sorting: e2 and e4 become the two pivots (2nd and 4th smallest)
- This guarantees P1 ≤ P2 without additional comparison

#### 2.5 Small Array Optimization — A Complete Story
**The Problem**: Recursion overhead dominates at small sizes. Function call overhead (~20 cycles), stack frame setup, and partition logic cost more than the actual sorting work for tiny arrays.

**Prior Work**: Sedgewick (1978) established that switching to insertion sort below a threshold improves quicksort performance. The optimal threshold depends on hardware characteristics.

---

##### 2.5.1 Design: Strategy Selection Based on Partition Position

The key insight (inherited from Java's DualPivotQuicksort) is that the **position** of a partition in the recursion tree determines which insertion strategy is optimal:

| Partition Position | Strategy | Threshold | Why? |
|--------------------|----------|-----------|------|
| **Leftmost** | Simple insertion + prefetch | < 60 | No sentinel — needs bounds checking |
| **Non-leftmost** | Mixed insertion (pin + pair) | < 60 + bits | Has sentinel — can skip bounds checking |

**Why Position Matters**:
- **Leftmost partition**: This is the actual start of the array. When inserting, we may reach `low` and must check bounds (`while (--i >= low)`).
- **Non-leftmost partition**: There is always a smaller element to the left (the pivot from the parent). This acts as a **sentinel** — the inner loop automatically stops without explicit bounds checking.

**Implementation** (sequential_sorters.hpp):
```cpp
// Non-leftmost: use mixed insertion (bits & 1 == 1)
if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
    mixed_insertion_sort(a, low, high, comp);
    return;
}
// Leftmost: use simple insertion
if (size < MAX_INSERTION_SORT_SIZE) {
    insertion_sort(a, low, high, comp);
    return;
}
```

---

##### 2.5.2 Simple Insertion Sort (Leftmost Partitions)
Used for **leftmost** partitions where no sentinel exists:
```cpp
for (std::ptrdiff_t i, k = low; ++k < high; ) {
    T ai = a[i = k];
    
    if (DPQS_LIKELY(k + 1 < high)) {
        DPQS_PREFETCH_READ(&a[k + 1]);  // Cache warming
    }
    
    if (DPQS_UNLIKELY(comp(ai, a[i - 1]))) {
        while (--i >= low && comp(ai, a[i])) {  // Must check bounds!
            a[i + 1] = a[i];
        }
        a[i + 1] = ai;
    }
}
```

**Empirical Finding — Prefetch Overhead**:
Our experiment (§2.5.6) reveals that prefetch **hurts** performance for arrays < 40 elements:
- For tiny arrays, data is already in L1 cache
- The prefetch instruction overhead outweighs any cache benefit
- However, the code keeps prefetch because insertions are rare in nearly-sorted data (the `DPQS_UNLIKELY` branch is rarely taken)

---

##### 2.5.3 Mixed Insertion Sort: Combining Pin and Pair Strategies

For **non-leftmost** partitions, `mixed_insertion_sort` uses a sophisticated two-phase approach:

**Phase Transition Formula** (inherited from Java):
```cpp
std::ptrdiff_t end = high - 3 * ((size >> 5) << 3);

if (end == high) {
    // Small array (size < 32): use simple insertion
} else {
    // Phase 1: Pin insertion for elements [low..end)
    // Phase 2: Pair insertion for elements [end..high)
}
```

**What This Formula Means**:
| Size Range | Formula Result | Pin Handles | Pair Handles |
|------------|----------------|-------------|--------------|
| 8–31 | `end == high` | None (simple insertion) | None |
| 32–63 | 24 elements at end | First 8-39 elements | Last 24 elements |
| 64–95 | 48 elements at end | First 16-47 elements | Last 48 elements |
| 96–127 | 72 elements at end | First 24-55 elements | Last 72 elements |

**Key Insight**: The inner boundary of **32** is where pin+pair kicks in — this is NOT independently tuned but inherited from Java.

---

##### 2.5.4 Pin Insertion Sort (Phase 1)
**The Idea**: Use a "pin" element to separate small and large values, reducing unnecessary comparisons for large elements.

**How It Works**:
```
Initial: [sorted part] [pin] [unsorted: small and large mixed]
                        ↑
                       pin separates small/large
```

1. **Select pin**: `pin = a[end]` — the element at the phase transition point
2. **For each element**:
   - If **< pin**: Insert into sorted part (standard insertion)
   - If **> pin**: Swap with element from the end, then insert the swapped (smaller) element

**Why This Helps**:
| Scenario | Simple Insertion | Pin Insertion |
|----------|------------------|---------------|
| Large element | Compares against ALL sorted elements | O(1) swap to end |
| Cache pattern | Random comparisons | Fewer memory accesses |

---

##### 2.5.5 Pair Insertion Sort (Phase 2)
After pin insertion handles the first portion, **pair insertion** processes the remainder two elements at a time.

**The Algorithm**:
```cpp
for (std::ptrdiff_t i; low < high; ++low) {
    T a1 = a[i = low], a2 = a[++low];  // Grab pair
    
    if (comp(a2, a1)) {
        // a1 > a2: Insert larger (a1) first, then smaller (a2)
        while (--i >= start && comp(a1, a[i])) a[i + 2] = a[i];
        a[++i + 1] = a1;
        while (--i >= start && comp(a2, a[i])) a[i + 1] = a[i];
        a[i + 1] = a2;
    } else if (comp(a1, a[i - 1])) {
        // a1 <= a2: Insert a2 first, then a1
        // ...
    }
    // else: Both already in position
}
```

**Why Insert Larger First?**
When a1 > a2:
1. Insert a1 deep into the sorted portion (shifts elements by 2 positions)
2. Insert a2 at a1's position or earlier (shifts only up to a1's position)

If we inserted a2 first, we'd shift elements twice over the same region.

---

##### 2.5.6 Empirical Evaluation: Insertion Sort Variants (NEW)

**Experiment**: Compare five insertion sort strategies across array sizes 8–80.

**Variants Tested**:
1. **Naive**: Basic insertion, no optimizations
2. **Prefetch**: Simple insertion with `__builtin_prefetch` + branch hints
3. **Pin only**: Only pin insertion phase
4. **Pair only**: Only pair insertion phase
5. **Mixed**: Pin + pair combined (current implementation)

**Results** (nanoseconds per sort, 100K iterations):

| Size | Naive | Prefetch | Pin | Pair | Mixed | **Winner** |
|------|-------|----------|-----|------|-------|------------|
| 8 | 16 | 20 | 15 | **13** | 14 | Pair |
| 16 | 28 | 38 | 30 | **21** | 26 | Pair |
| 24 | 58 | 68 | 50 | **42** | 58 | Pair |
| 32 | 102 | 117 | 108 | 76 | **73** | Mixed |
| 44 | 233 | 184 | 137 | 132 | **131** | Mixed |
| 56 | 308 | 292 | 217 | 230 | **212** | Mixed |
| 64 | 397 | 398 | 400 | **248** | 248 | Pair/Mixed |
| 80 | 654 | 629 | 370 | **362** | 375 | Pair |

**[PLACEHOLDER: Figure 2.5.1 — Runtime Comparison of Insertion Sort Variants]**
*X-axis: Array size, Y-axis: Time per sort (ns). Shows crossover at size 32 where mixed becomes optimal.*

**Key Findings**:

1. **Prefetch Hurts Small Arrays**: For size < 40, prefetch is 15-37% **slower** than naive due to instruction overhead. The benefit only appears when data exceeds L1 cache.

2. **Pair Insertion Dominates**: As a standalone strategy, pair insertion is fastest for 60% of sizes tested. Processing two elements at a time genuinely reduces overhead.

3. **Mixed Optimal at 32+**: The combined pin + pair strategy (mixed) achieves 1.5–1.7× speedup over prefetch insertion for sizes ≥ 32.

4. **Inner Boundary is Reasonable**: The Java-inherited threshold of 32 (where pin+pair activates inside mixed insertion) aligns with the crossover point in our data.

---

##### 2.5.7 Tuning: Outer Threshold (Insertion vs Quicksort)

**Experiment**: Sweep MAX_INSERTION_SORT_SIZE from 10 to 80 on 10M element arrays.

**Results**:
| Threshold | Runtime (ms) | Notes |
|-----------|--------------|-------|
| 30 | 566 | |
| 45 | 573 | |
| 55 | 569 | |
| **60** | **560** | **Optimal** |
| 65 | 579 | Regression |

**Design Decision**: MAX_INSERTION_SORT_SIZE = 60 (both simple and mixed thresholds unified).

---

##### 2.5.8 Empirical Validation: The Adaptive Approach Works

The inner boundary (32) and strategy ordering (pin-then-pair) were inherited from Java's DualPivotQuicksort. Rather than claiming to have independently derived these values, we conducted experiments to **validate that the adaptive approach yields good efficiency** in C++.

---

**Experiment 1: Does Adapting Strategy by Size Help?**

We compared simple insertion vs. mixed (pin+pair) across the transition region:

| Size | Simple (ns) | Mixed (ns) | Speedup | Winner |
|------|-------------|------------|---------|--------|
| 16 | 63.1 | 55.3 | 1.14× | Mixed |
| 24 | 83.1 | 81.4 | 1.02× | Mixed |
| 28 | 95.9 | 88.1 | 1.09× | Mixed |
| **32** | **109.7** | **73.6** | **1.49×** | **Mixed** |
| 40 | 141.4 | 109.1 | 1.30× | Mixed |
| 48 | 198.0 | 132.4 | 1.50× | Mixed |
| 64 | 401.2 | 239.9 | 1.67× | Mixed |

**Finding**: The mixed strategy (pin+pair) beats simple insertion at **all tested sizes**, with speedups ranging from 1.02× to 1.67×. The benefit increases with array size, confirming that the adaptive approach is worthwhile.

**On the Specific Value of 32**: While our data shows the speedup crosses the 1.5× threshold around size 32, we do not claim this is the uniquely optimal boundary. The value 32 was inherited from Java, where it was presumably tuned through extensive empirical work. Our experiments confirm it remains a **reasonable choice in C++** — the adaptation delivers consistent speedups, and the boundary sits in the transition zone where benefits become substantial.

---

**Experiment 2: Why Pin FIRST, then Pair?**

We compared the ordering of insertion strategies:

| Size | Pin→Pair (ns) | Pair→Pin (ns) | Ratio | Winner |
|------|---------------|---------------|-------|--------|
| 32 | 64.0 | 80.2 | 1.25× | Pin→Pair |
| 40 | 116.2 | 127.9 | 1.10× | Pin→Pair |
| 48 | 141.7 | 175.3 | 1.24× | Pin→Pair |
| 56 | 212.6 | 245.5 | 1.15× | Pin→Pair |
| 64 | 250.2 | 320.2 | 1.28× | Pin→Pair |

**Finding**: Pin→Pair is consistently **10–28% faster** than Pair→Pin across all tested sizes.

**Interpretation**: 
- **Pin insertion** uses a cutoff element to quickly partition values without full comparisons. This creates a *roughly sorted* initial region.
- **Pair insertion** then processes remaining elements two at a time, benefiting from the structure left by pin insertion.
- If reversed, pair insertion would attempt to process unsorted data, followed by pin insertion on a partially-sorted array — suboptimal because pin's "large element swap" optimization provides less benefit on an already-structured region.

---

**Experiment 3: Why Use BOTH Strategies?**

We compared using pin only, pair only, or the combined mixed strategy:

| Size | Pin Only (ns) | Pair Only (ns) | Mixed (ns) | Winner |
|------|---------------|----------------|------------|--------|
| 32 | 96.4 | 65.5 | 65.8 | Pair |
| 40 | 143.7 | 110.3 | 114.3 | Pair |
| 48 | 154.8 | 152.7 | **134.1** | **Mixed** |
| 56 | 219.5 | 200.7 | 205.8 | Pair |
| 64 | 350.4 | 247.7 | **243.2** | **Mixed** |

**Finding**: 
- At **smaller sizes (32–40)**, pair-only is competitive or slightly better.
- At **larger sizes (48–64)**, the **combined mixed strategy wins**.

**Interpretation**: Pin insertion's "swap large elements to end" heuristic becomes more valuable as array size increases — there are more opportunities for early termination. The formula `3 * ((size >> 5) << 3)` adaptively allocates more work to pair insertion as size grows, achieving the correct balance.

---

**Experiment 4: Optimal Pair-Count Verification**

For size=48, we swept the number of elements handled by pair insertion:

| pair_count | pin_count | Runtime (ns) |
|------------|-----------|--------------|
| 0 | 48 | 195.8 |
| 8 | 40 | 159.9 |
| 16 | 32 | 163.1 |
| 20 | 28 | 144.6 |
| **24** | **24** | **139.7** ← Java default |
| 28 | 20 | 155.8 |
| 36 | 12 | 143.0 |
| 48 | 0 | 197.8 |

**Finding**: Java's default (`pair_count = 24` from formula `3 * ((48 >> 5) << 3) = 3 × 8 = 24`) is **empirically optimal**. The 50/50 split minimizes total runtime.

---

##### 2.5.9 Summary: Small Array Optimization Design

| Design Choice | Source | Empirical Status |
|---------------|--------|------------------|
| Outer threshold = 60 | Tuned for C++ | **Optimal** (§2.5.7) |
| Inner boundary = 32 | Java inheritance | **Retained** — adaptive approach validated |
| Pin→Pair ordering | Java inheritance | **Validated** — 10-28% faster than reverse |
| Combined pin+pair | Java inheritance | **Validated** — optimal for size ≥ 48 |
| Pair_count formula | Java inheritance | **Validated** — matches empirical optimum |

**Key Takeaway**: While we cannot fully justify why 32 is the specific optimal boundary, our experiments demonstrate that **the adaptive strategy works well**. Switching to pin+pair at size 32 yields consistent speedups (1.5×–1.7× faster than simple insertion for sizes ≥ 32), and the formula-based allocation of work between pin and pair matches the empirically optimal split. The inherited constants from Java remain suitable for C++.

**Remaining Future Work**:
| Opportunity | Status |
|-------------|--------|
| Platform-specific tuning (AVX-512, ARM) | Not yet performed |
| Fine-grained boundary sweep (e.g., 28 vs 32 vs 36) | Could yield minor improvements |

#### 2.6 Recursion Safety and Heap Sort Fallback
**The Problem**: Stack overflow on deeply recursive sorts (adversarial input). Adversarial inputs (e.g., all equal elements with broken comparator, or crafted "anti-quicksort" sequences) can force O(n²) partitions, causing stack overflow before completion.

**Design**:
- Tail call optimization: Process smallest partition inline, recurse on largest
- Depth limiting: Heapsort fallback at MAX_RECURSION_DEPTH = 64 × DELTA = 192

**Implementation** (heap_sort.hpp):
- **Push-down heapify**: Bottom-up heap construction in O(n)
- **Extract-max loop**: Repeatedly swap root to end, restore heap property
- **Why heapsort?**: Guaranteed O(n log n) worst case, O(1) auxiliary space

**The Fallback Trigger**:
```cpp
if (bits > MAX_RECURSION_DEPTH) {
    heap_sort(a, low, high, comp);  // Guaranteed O(n log n)
    return;
}
```

**Why This Matters**:
- Converts Introsort's "detect and switch" pattern to DPQS
- Without fallback: Adversarial input → stack overflow or O(n²)
- With fallback: All inputs complete in O(n log n), bounded stack

**Trade-off**:
| Aspect | Quicksort | Heapsort |
|--------|-----------|----------|
| Average case | O(n log n) | O(n log n) |
| Constant factor | Lower (~1.4n log n) | Higher (~2n log n) |
| Cache behavior | Excellent (sequential) | Poor (jumping) |
| Worst case | O(n²) without fallback | **O(n log n) guaranteed** |

**Design Decision**: Use heapsort as the "insurance policy" — rarely triggered (<0.01% of real data), but guarantees robustness

---

### Chapter 3: Adaptive Optimizations (10-12 pages)

This chapter presents two major adaptive optimization stories: the Run Merger (achieving 19× speedup on structured data) and Type-Specific Paths (O(n) for small integer types).

#### 3.1 Run Merger: Exploiting Sorted Runs — The Hero Feature

##### 3.1.1 Prior Work: Timsort and Adaptive Sorting

**The Timsort Revolution** (Peters, 2002):
Tim Peters designed Timsort for Python, recognizing that real-world data is rarely random:
- **Key insight**: Detect existing sorted "runs" and merge them
- **Complexity**: O(n) for already-sorted data, O(n log n) for random
- Adopted by Python (2002), Java (Arrays.sort for objects), Android

**Java's Adaptation** (DualPivotQuicksort.java):
Java combined Timsort's run detection with dual-pivot quicksort:
- Scan for runs at array start
- Quality heuristics decide: merge path vs quicksort path
- Best of both worlds: Fast on structured AND random data

**Our Implementation**: Port Java's hybrid approach to C++, with platform-specific tuning.

##### 3.1.2 The Problem
Many real-world datasets have pre-existing order:
- Database records arrive mostly sorted
- Log files have timestamps in order
- User-generated content preserves partial ordering

Standard quicksort ignores this structure and re-partitions everything — essentially "un-sorting" the already-sorted segments before sorting them again.

**The Opportunity**: Detect pre-existing runs and merge them directly → O(n) instead of O(n log n).

##### 3.1.3 Design: Run Detection Mechanism
**Core Mechanism** (from Java's DualPivotQuicksort):
1. Scan for ascending/descending runs at array start
2. Check run quality against heuristics
3. If quality passes → merge runs; otherwise → fall back to quicksort

**Quality Heuristics**:
| Parameter | Java Value | Our Value | Purpose |
|-----------|------------|-----------|---------|
| MIN_FIRST_RUN_SIZE | 16 | 16 | Minimum length for first run |
| MIN_FIRST_RUNS_FACTOR | 7 | **6** | Controls minimum run length relative to array size |
| MAX_RUN_CAPACITY | 500 | 500 | Maximum runs before fallback |
| MIN_RUN_COUNT | 5 | 5 | Minimum runs for parallel merge |

##### 3.1.4 Implementation

**Run Detection** (run_merger.hpp):
- Ascending, descending (reversed), and constant run handling
- Early termination: Already sorted detection in O(n)
- Merge tree construction for efficient run combination
- Parallel merge when parallel && count >= MIN_RUN_COUNT

**Sequential Merge** (merge_ops.hpp):
The two-pointer merge is deceptively simple but critical for performance:
```cpp
while (lo1 < hi1 && lo2 < hi2) {
    dst[k++] = comp(a1[lo1], a2[lo2]) ? a1[lo1++] : a2[lo2++];
}
// Copy remaining elements from whichever array isn't exhausted
```

**Why This Matters**:
- Branch-free inner loop: Modern CPUs predict the ternary well
- Sequential memory access: Perfect for hardware prefetcher
- No auxiliary comparisons: Single compare per element moved

**Buffer Management** (buffer_manager.hpp):
**The Problem**: Merge requires O(n) auxiliary space. Naive allocation (malloc per merge) would add thousands of allocations during parallel execution.

**Solution**: Thread-local buffer pooling
```cpp
template<typename T>
class BufferManager {
    static thread_local std::vector<T> buffer_pool;
    static T* getBuffer(int size, int& offset);
    static void returnBuffer(T* buffer, int size, int offset);
};
```

**Key Design Decisions**:
| Feature | Benefit |
|---------|--------|
| Thread-local | No mutex contention between threads |
| Pooling | Amortize allocation cost over many merges |
| Offset tracking | Reuse same buffer for non-overlapping merges |
| Geometric growth | Minimize reallocations (1.5× growth factor) |

**Impact**:
- Without pooling: ~15,000 malloc/free calls for 10M element parallel sort
- With pooling: ~16 allocations (one per thread, occasionally resized)
- **Result**: 8-12% speedup on parallel merge-heavy workloads

##### 3.1.5 Tuning: MIN_FIRST_RUNS_FACTOR Optimization

**Background: The Run Detection Optimization**

Real-world data is rarely truly random. Database records arrive mostly sorted, log files have timestamps in order, and user-generated content often preserves partial ordering. Standard quicksort ignores this structure and re-partitions everything — essentially "un-sorting" the already-sorted segments before sorting them again.

The run merger optimization (inspired by Timsort) detects these pre-existing sorted segments ("runs") and merges them directly, achieving near-linear O(n) time for structured data versus O(n log n) for random data. This is why our implementation achieves **19× speedup on ORGAN_PIPE** and **7× on REVERSE_SORTED** patterns.

**The Trade-off**:
- **Merge path**: O(n) for well-structured data, but requires auxiliary buffer
- **Quicksort path**: O(n log n) always, but no extra memory needed

**The Key Heuristic**:
- MIN_FIRST_RUNS_FACTOR = 7 → minimum run ≈ 14.3% of array
- MIN_FIRST_RUNS_FACTOR = 6 → minimum run ≈ 16.7% of array

Higher factor = more aggressive (triggers merge on shorter runs)
Lower factor = more conservative (requires longer runs)

**Hypothesis**:
Java's default of 7 was tuned for JVM performance characteristics. C++ with -O2 -march=native may have different crossover points due to:
- Different memory allocation costs
- Different function call overhead
- Different branch prediction behavior

**Test Configuration**:
- Array size: 1M integers
- Run lengths: 16, 32, 64, 128, 256, 512, 1024
- Paths: Force merge vs Force quicksort
- Metric: Median runtime over 10 iterations

**Results**:
| Run Length | Force Merge (ms) | Force Quicksort (ms) | Winner | Margin |
|------------|------------------|----------------------|--------|--------|
| 16 | 89 | 78 | **Quicksort** | +14% |
| 32 | 82 | 78 | **Quicksort** | +5% |
| **64** | **76** | **78** | **Merge** | **-2.6%** |
| 128 | 71 | 78 | Merge | -9% |
| 256 | 65 | 78 | Merge | -17% |
| 512 | 58 | 78 | Merge | -26% |
| 1024 | 52 | 78 | Merge | -33% |

**Key Finding**: The crossover point is between run length 32 and 64.

**Why Change from 7 to 6?**
| Scenario | Factor=7 | Factor=6 |
|----------|----------|----------|
| Short runs (32) | Would merge (wasteful) | **Rejects**, uses quicksort |
| Medium runs (64) | Would merge | Would merge (correctly) |
| Long runs (128+) | Would merge | Would merge |

**Validation Results**:
| Pattern | Factor=7 (Java) | Factor=6 (Tuned) | Change |
|---------|-----------------|------------------|--------|
| RANDOM | 480 ms | 480 ms | 0% |
| NEARLY_SORTED | 52 ms | 52 ms | 0% |
| SAWTOOTH (short runs) | 68 ms | **65 ms** | **-4.4%** |
| SAWTOOTH (long runs) | 41 ms | 41 ms | 0% |
| ORGAN_PIPE | 28 ms | 28 ms | 0% |

**Design Decision**: Change MIN_FIRST_RUNS_FACTOR from 7 to 6. This makes the run quality assessment slightly stricter, correctly rejecting short-run data that would regress under merge-based sorting.

**Lesson Learned**:
Inherited constants from Java deserve re-evaluation in C++ context. While Java's values are well-tuned for JVM characteristics, C++'s different performance profile can shift optimal crossover points. However, changes should be conservative and backed by empirical evidence.

##### 3.1.6 Result
- **19× speedup on ORGAN_PIPE** (ascending then descending)
- **6× speedup on REVERSE_SORTED** (single descending run)
- **10× speedup on SAWTOOTH** (multiple sorted chunks)
- Transforms O(n log n) problem into O(n) by recognizing inherent structure

---

#### 3.2 Counting Sort for Small Integer Types

##### 4.2.1 The Opportunity
1-byte and 2-byte integral types have bounded range (256 or 65536 values). Instead of O(n log n) comparison sort, we can achieve O(n) via bucket counting.

##### 4.2.2 Implementation (counting_sort.hpp)
- Signed/unsigned offset calculation for index mapping
- Sparse vs Dense optimization: Different iteration direction based on fill ratio
  - Dense (size > 128): Iterate backward, fill from end
  - Sparse (size ≤ 128): Skip zero buckets, fill from start

**Threshold Analysis**:
- Byte types: Threshold 64
- Short/char types: Threshold 1750
- Rationale: Frequency array overhead must be amortized

---

#### 3.3 Floating-Point Edge Cases (float_sort.hpp)

##### 3.3.1 IEEE-754 Challenges
- **NaN**: NaN != NaN breaks standard comparison
- **Negative zero**: -0.0 == +0.0 mathematically, but need consistent ordering

##### 3.3.2 Solution
- **Preprocessing**: Move NaNs to end via value != value check; convert -0.0 to +0.0
- **Postprocessing**: Binary search to restore -0.0 positions using std::signbit()

---

### Chapter 4: Parallel Execution (10-12 pages)

This chapter presents the complete parallel implementation story: from the work-stealing thread pool through performance tuning to understanding the fundamental hardware limits.

#### 4.1 Work-Stealing Thread Pool — A Complete Story

##### 4.1.1 Prior Work: Parallel Sorting and Work-Stealing

**The Work-Stealing Paradigm** (Blumofe & Leiserson, 1999):
- **Problem**: Recursive algorithms create imbalanced work trees
- **Solution**: Idle threads "steal" work from busy threads' queues
- **Key property**: LIFO local access (cache locality) + FIFO stealing (large tasks first)
- **Theoretical guarantee**: O(T₁/P + T∞) expected time with P processors

**Implementations Studied**:
| System | Key Feature | Limitation |
|--------|------------|------------|
| Intel TBB | Auto-partitioner | Requires library dependency |
| Java ForkJoinPool | CountedCompleter pattern | JVM-specific |
| C++ std::async | Simple API | No work-stealing (creates threads) |

**Our Approach**: Custom work-stealing pool inspired by ForkJoinPool, with C++ optimizations (try_lock, cache-line padding, sticky victim).

**The Memory Wall** (Wulf & McKee, 1995):
- CPU speed grows faster than memory bandwidth
- Sorting is memory-bound: data movement dominates computation
- **Implication**: Parallel speedup limited by shared memory bandwidth, not CPU count

##### 4.1.2 The Problem
Recursive sorting creates imbalanced work:
- Initial partition divides into 3 unequal regions
- Static thread assignment leads to idle threads
- Need dynamic load balancing without central bottleneck

##### 4.1.3 Design Evolution: Three Generations

**Phase V1: Single Global Mutex**
- Simple implementation: All tasks in one queue
- Problem: Severe contention at high thread counts
- Result: Scaling plateau at 2-3× speedup

**Phase V2: Per-Thread Queues with Central Dispatch**
- Each thread has local queue
- Central dispatcher assigns tasks
- Problem: Dispatcher becomes bottleneck
- Result: Better, but still limited to 3-4× speedup

**Phase V3: Work-Stealing with LIFO/FIFO (Final)**
- Distributed WorkStealingQueue per thread (no global mutex)
- LIFO local access (cache locality) + FIFO stealing (take largest tasks)
- try_lock for non-blocking steal attempts
- Result: Achieved 5.18× speedup on 16 threads

##### 4.1.4 Implementation Details

**Thread Pool Design** (threadpool.hpp):
- Distributed WorkStealingQueue per thread
- LIFO local access: Pop from bottom (most recent, likely in cache)
- FIFO stealing: Steal from top (oldest, largest partitions)
- try_lock for non-blocking steal attempts

**CountedCompleter Pattern** (completer.hpp):
- Ported from Java's ForkJoinPool
- Pending counter with atomic fetch_add for child registration
- Completion propagation via condition_variable
- Exception handling with completeExceptionally()

**Type Erasure System** (types.hpp):
Java's polymorphism allows `Object[]` to hold any array type. C++ templates don't work this way — generic parallel coordination requires type erasure.

**The Problem**: Parallel merge tasks need to pass array pointers through a generic task queue, but C++ templates instantiate separate types for `int*`, `double*`, etc.

**Solution**: `std::variant` + wrapper class
```cpp
using ArrayVariant = std::variant<
    int*, long*, float*, double*,
    int8_t*, int16_t*, uint8_t*, uint16_t*, ...
>;

struct ArrayPointer {
    ArrayVariant data;
    
    template<typename T> ArrayPointer(T* ptr) : data(ptr) {}
    template<typename T> T* get() const { return std::get<T*>(data); }
    template<typename T> bool is() const { 
        return std::holds_alternative<T*>(data); 
    }
};
```

**Why This Matters**:
- **Type safety**: Compile-time checked, unlike `void*`
- **Visitor pattern**: `std::visit` enables generic operations
- **Zero overhead**: Variant is stack-allocated, no heap indirection
- **Java equivalence**: Matches `instanceof` checks in Java's RunMerger

**Usage in Parallel Merge**:
```cpp
class GenericMerger : public CountedCompleter<void> {
    ArrayPointer dst, a1, a2;  // Works with any supported type
    void compute() override {
        dst.visit([&](auto* d) {
            // d is correctly typed (int*, double*, etc.)
            merge_parts(d, k, a1.get<...>(), ...);
        });
    }
};
```

##### 4.1.5 Optimizations Applied

**Phase 1: Adaptive Granularity**
- Problem: Static sequential cutoff ignores runtime load
- Solution: Dynamically double threshold when queue depth > 4×threads
- Metric: get_active_task_count() atomic load

**Phase 2: Memory-Aware Scheduling (Sticky Victim)**
- Problem: Random stealing causes cache thrashing
- Solution: Remember last successful victim, prefer spatial locality
- Result: Improved L2/L3 cache hit rates

**Phase 3: Hybrid Parallelism (Depth Cutoff)**
- Problem: Excessive recursion at tree leaves adds overhead
- Solution: Force sequential sort when depth > 20 levels
- Result: Broke 4.4× plateau → achieved 5.18× speedup

##### 4.1.6 Task Granularity Management
- MIN_PARALLEL_SORT_SIZE=8192 (base threshold)
- Adaptive: Double threshold when active_tasks > 4×threads
- Hybrid: Force sequential after 20 recursion levels

---

### Chapter 5: Results and Evaluation (10-12 pages)

This chapter presents comprehensive benchmarking results comparing our dual-pivot quicksort implementation against std::sort across diverse data patterns, array sizes, and thread configurations.

#### 5.1 Experimental Setup

##### 5.1.1 Hardware Platform
| Component | Specification |
|-----------|---------------|
| **CPU** | Intel Core i7-13700 (Raptor Lake), 10 nm process |
| **Cores / Threads** | 8 Performance + 8 Efficiency (16 cores, 24 threads) |
| **L2 Cache** | P-cores: 8×2 MB; E-cores: 2×4 MB (cluster shared) |
| **L3 Cache** | 30 MB (shared) |
| **RAM** | 32 GB DDR5-4800 (dual-channel) |

##### 5.1.2 Software Environment
| Component | Version |
|-----------|---------|
| **Operating System** | Windows 11 Pro |
| **Compiler** | g++ 13.2.0 (MinGW-w64) |
| **Optimization Flags** | -O2 -march=native -DNDEBUG |
| **C++ Standard** | C++17 |
| **Profiler** | Intel VTune Profiler 2025.10 |

**Compiler Flag Selection**: We benchmarked 12 GCC flag combinations. Key findings:
- `-O3` offers no benefit over `-O2` for branch-heavy sorting code
- `-flto` causes 2-5% regression (header-only templates already get full inlining)
- `-march=native` provides ~1.5% improvement (memory-bound workload limits gains)

Final choice: `-O2 -march=native` — simplest flag set achieving best performance.

##### 5.1.3 Benchmark Protocol
1. **Warmup Phase**: 3 iterations discarded
2. **Measurement Phase**: 10 timed iterations
3. **Statistical Reporting**: Median runtime
4. **Timing Method**: std::chrono::high_resolution_clock

##### 5.1.4 Test Matrix
| Parameter | Values |
|-----------|--------|
| **Array Sizes** | 1K, 10K, 100K, 1M, 10M elements |
| **Data Patterns** | RANDOM, REVERSE_SORTED, ORGAN_PIPE, SAWTOOTH, NEARLY_SORTED, MANY_DUPLICATES |
| **Thread Counts** | 1, 2, 4, 8, 16 |

Total: 5 sizes × 6 patterns × 5 thread counts = **150 configurations**

##### 5.1.5 Data Pattern Relevance
| Pattern | Real-World Source | Example |
|---------|------------------|---------|
| RANDOM | Hash table outputs | User IDs after hashing |
| NEARLY_SORTED | Incremental updates | Database with new inserts |
| REVERSE_SORTED | Opposite key sort | Price high→low needs low→high |
| MANY_DUPLICATES | Categorical data | Star ratings (1-5) |
| ORGAN_PIPE | Time series peaks | Stock prices over day |
| SAWTOOTH | Sorted chunks | Merging log files |

##### 5.1.6 Reproducibility
- **Source code**: include/dual_pivot_quicksort.hpp
- **Benchmark runner**: benchmarks/benchmark_runner.cpp
- **Raw results**: benchmarks/results/

#### 5.2 Performance by Data Pattern

##### 5.2.1 Random Data
**[PLACEHOLDER: Figure 6.2.1 — Random Data Performance]**

**Analysis**: Sequential DPQS within 5% of std::sort. Parallel achieves 5.18× speedup (1T→16T).

##### 5.2.2 Reverse-Sorted Data
**Algorithm Trigger**: run_merger.hpp detects single descending run.
**Mechanism**: O(n) in-place reversal.
**Result**: ~6× speedup vs std::sort

**[PLACEHOLDER: Figure 6.2.2 — REVERSE_SORTED Pattern]**

##### 5.2.3 Organ-Pipe Data
**Algorithm Trigger**: run_merger.hpp detects 2 runs.
**Mechanism**: O(n) merge of ascending + reversed descending.
**Result**: **19× speedup** — largest across all patterns

**[PLACEHOLDER: Figure 6.2.3 — ORGAN_PIPE Pattern]**

##### 5.2.4 Sawtooth Data
**Algorithm Trigger**: run_merger.hpp detects k ascending runs.
**Mechanism**: O(n log k) merge tree, parallelized.
**Result**: ~10× speedup, best parallel scaling

**[PLACEHOLDER: Figure 6.2.4 — SAWTOOTH Pattern]**

##### 5.2.5 Nearly-Sorted Data
**Algorithm Trigger**: Quality heuristics determine path.
**Key Insight**: Tests MIN_FIRST_RUNS_FACTOR tuning.

**[PLACEHOLDER: Figure 6.2.5 — NEARLY_SORTED Pattern]**

##### 5.2.6 Duplicate-Heavy Data
**Adaptive Pivot Strategy**: Dutch National Flag on duplicates.
**Result**: No degradation; normal parallel scaling.

**[PLACEHOLDER: Figure 6.2.6 — MANY_DUPLICATES Pattern]**

#### 5.3 Parallel Scaling Analysis

##### 5.3.1 Speedup Results (VTune Measured)
| Threads | Runtime (ms) | Speedup | Efficiency | CPI | Primary Bottleneck |
|---------|--------------|---------|------------|-----|-------------------|
| 1 | 508 | 1.00x | 100% | 0.889 | Branch Mispredict (35%) |
| 2 | 265 | 1.92x | 96% | 0.855 | Branch Mispredict (36%) |
| 4 | 154 | 3.30x | 82% | 0.919 | Branch Mispredict (32%) |
| 8 | 110 | 4.62x | 58% | 1.134 | L3 Cache (19%) + Branch (29%) |
| 16 | 98 | 5.18x | 32% | 1.729 | L3 Cache (38%) + Sync (48%) |

**Key Observation**: Bottleneck shifts from branch misprediction to L3 cache contention.

##### 5.3.2 VTune Bottleneck Analysis

**Pipeline Slot Breakdown (16 Threads)**:
| Category | P-core % | Impact |
|----------|----------|--------|
| Memory Bound | 41.1% | PRIMARY |
| └── L3 Bound | 37.9% | Cache line thrashing |
| Bad Speculation | 25.7% | Branch misprediction |
| Front-End Bound | 19.8% | Instruction fetch |
| Retiring (Useful Work) | 9.8% | Actual computation |

**Critical Finding**: Only 9.8% of pipeline slots perform useful work at 16 threads.

**Synchronization Overhead**:
| Function | CPU Time % | Cause |
|----------|-----------|-------|
| sched_yield | 37.8% | Thread waiting |
| pthread_mutex_trylock | 7.3% | Work-stealing locks |
| partition_dual_pivot | 26.8% | Sorting work |

**Conclusion**: VTune confirms scaling plateau is caused by L3 cache contention (38%) and synchronization (48% spin time) — hardware, not software, is the bottleneck.

##### 5.3.3 Amdahl's Law Application
**Serial fraction**: ~13.9% (from 5.18×@16T)
**Maximum theoretical speedup**: 7.19×

**VTune-Identified Serial Overhead**:
| Component | Impact |
|-----------|--------|
| Initial partitioning | ~4% |
| Synchronization spin | ~6% |
| Memory serialization | ~3% |
| Work-stealing overhead | ~1% |

##### 5.3.4 VTune-Guided Optimizations
**Successful**:
1. Task granularity adjustment (MIN_PARALLEL_SORT_SIZE = 65536)
2. Cache-line padding (alignas(64) on atomics)
3. Software prefetching

**Unsuccessful**:
| Attempt | Result |
|---------|--------|
| Chase-Lev lock-free deque | 20-58% regression |
| Batch classification | 30% slower |
| Block partitioning | Negative |

#### 5.4 Space Complexity Analysis

| Algorithm | Random Input | Structured Input | Worst Case |
|-----------|--------------|------------------|------------|
| std::sort | O(log n) | O(log n) | O(log n) |
| DPQS | O(log n) | **O(n)** | **O(n)** |
| Timsort | O(n) | O(n) | O(n) |
| pdqsort | O(log n) | O(log n) | O(log n) |

**Trade-off**: DPQS uses O(n) auxiliary space for structured data to achieve 19× speedup.

#### 5.5 Correctness Verification

##### 5.5.1 Test Suite (16 test files)
| Category | Purpose |
|----------|---------|
| Core Algorithm | Dual-pivot partitioning invariants |
| Specialized Paths | Counting sort, float sort, heap sort |
| Adaptive Behavior | Run detection heuristics |
| Parallel Infrastructure | Thread coordination |
| Integration | End-to-end with all patterns |

##### 5.5.2 Edge Cases Covered
- Empty array, single element, all duplicates
- INT_MIN, INT_MAX, mixed extremes
- IEEE-754: NaN, -0.0, ±∞, denormals

##### 5.5.3 Fuzz Testing
- stress_test_manager.py: Thousands of random inputs
- All types: int8-int64, uint variants, float, double
- Results: No failures (stress_failures/ empty)

---

### Chapter 6: Discussion (4-5 pages)

#### 6.1 Interpretation of Results
##### 6.1.1 Why Structured Data Shows Dramatic Speedups (up to 19x)
- Key mechanism: run_merger.hpp detects existing runs before partitioning
- std::sort (Introsort) has NO run detection
- This is Timsort's key innovation, adopted by Java's DPQS
- Trade-off: O(n) auxiliary space vs O(log n) for pure quicksort

##### 6.1.2 Memory Wall Explanation for Parallel Scaling Plateau
- Sorting is **memory-bound**, not compute-bound
- Bandwidth saturation at ~4 threads on test hardware
- This is a **fundamental hardware limitation**, not software deficiency
- Evidence: All parallel sorting algorithms hit similar walls

#### 6.2 Comparison with Related Work
##### 6.2.1 vs Java's DualPivotQuicksort
- C++ port with manual memory management, custom thread pool, templates
- Performance parity achieved

##### 6.2.2 vs pdqsort
- Different strategy: single pivot with pattern detection
- DPQS wins on structured data; pdqsort wins on random

##### 6.2.3 vs std::sort
- DPQS dominates on structured data (run merging)
- std::sort competitive on random (both O(n log n), highly tuned)

#### 6.3 Practical Implications
- Use DPQS when data likely has patterns
- Use std::sort for guaranteed space bounds

#### 6.4 Limitations
- O(n) space for structured data
- Platform-specific tuning required
- No SIMD vectorization (future work)

---

### Chapter 7: Conclusion and Future Work (2-3 pages)

#### 7.1 Summary of Achievements
1. ✅ Complete C++ implementation of dual-pivot quicksort
2. ✅ STL-compatible header-only library
3. ✅ Parallel work-stealing implementation (5.18× speedup)
4. ✅ Up to 19× speedup on structured data vs std::sort
5. ✅ Comprehensive benchmarking across 6 data patterns
6. ✅ Empirical constant tuning with documented methodology

#### 7.2 Contributions
- First open-source, production-quality C++ DPQS implementation
- Empirical evidence that sorting is memory-bandwidth limited
- Algorithm engineering case study with documented trade-offs

#### 7.3 Future Work
- AVX2/AVX-512 vectorization (non-temporal stores)
- Block-based partitioning (BlockQuicksort approach)
- Adaptive parallel/sequential switching based on system load
- Multi-language ports (Python, Rust, Go bindings)

---

## References (~2 pages)
- Yaroslavskiy, V. (2009). Dual-Pivot Quicksort
- Wild, S. (2012). Why Is Dual-Pivot Quicksort Fast?
- Blumofe, R.D. & Leiserson, C.E. (1999). Work-Stealing
- Musser, D.R. (1997). Introsort
- Amdahl, G.M. (1967). Validity of the Single Processor Approach
- Wulf & McKee (1995). Hitting the Memory Wall
- Hoare, C.A.R. (1962). Quicksort. The Computer Journal, 5(1), 10-16
- Sedgewick, R. (1978). Implementing Quicksort Programs. Comm. ACM, 21(10), 847-857
- Bentley, J.L. & McIlroy, M.D. (1993). Engineering a Sort Function. Software: Practice and Experience, 23(11), 1249-1265
- Bose, R.C. & Nelson, R.J. (1962). A Sorting Problem. JACM, 9(2), 282-296

---

## Appendices (Not counted in 50-page limit)

### Appendix A: Source Code Listings
- Key algorithm implementations (partitioning, work-stealing)

### Appendix B: Complete Benchmark Data
- Full CSV results tables

### Appendix C: Tuning Experiment Raw Data
- Sweep results for each constant

### Appendix D: Build and Usage Instructions
- Compilation commands
- API examples

---

## Page Budget Estimate

| Chapter | Pages | Notes |
|---------|-------|-------|
| Chapter 1: Introduction | 5 | Includes §1.5 Related Work Overview |
| Chapter 2: Core Algorithm | 10 | Prior Work integrated (§2.1) |
| Chapter 3: Adaptive Optimizations | 11 | Prior Work on Timsort (§3.1.1) |
| Chapter 4: Parallel Execution | 6 | Work-stealing thread pool only |
| Chapter 5: Results and Evaluation | 10 | |
| Chapter 6: Discussion | 4 | |
| Chapter 7: Conclusion | 3 | |
| **Total (Main Body)** | **~49 pages** | Within 50-page limit |

**Benefits of Integrated Literature**:
- Saved ~6 pages from standalone Chapter 2
- Each citation now appears in context of the problem it addresses
- Readers understand "why" before seeing "what"
