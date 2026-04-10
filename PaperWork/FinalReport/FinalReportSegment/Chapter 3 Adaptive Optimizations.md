# Chapter 3: Adaptive Optimizations

This chapter presents two classes of adaptive optimization: run-aware sorting that exploits pre-existing structure (achieving up to 19× speedup), and type-specific paths that leverage bounded ranges for O(n) performance on small integer types.

---

## 3.1 Run Merger: Exploiting Sorted Runs

### 3.1.1 Prior Work: Timsort and Adaptive Sorting

Tim Peters designed Timsort for Python in 2002, recognizing that real-world data is rarely random. The key insight: detect existing sorted "runs" and merge them, achieving O(n) for already-sorted data versus O(n log n) for random input. This approach was adopted by Python, Java (for object arrays), and Android.

Java's `DualPivotQuicksort` combined Timsort's run detection with dual-pivot quicksort:
1. Scan for ascending/descending runs at array start
2. Apply quality heuristics to decide: merge path vs quicksort path
3. Achieve fast performance on both structured and random data

Our implementation ports Java's hybrid approach to C++, with platform-specific tuning of the quality thresholds.

### 3.1.2 The Problem

Many real-world datasets exhibit pre-existing order:
- Database records arrive mostly sorted by primary key
- Log files contain timestamps in chronological order
- User-generated content often preserves partial ordering

Standard quicksort ignores this structure, re-partitioning everything—essentially "un-sorting" the already-sorted segments before sorting them again. The opportunity: detect pre-existing runs and merge them directly for O(n) instead of O(n log n).

### 3.1.3 Design: Run Detection Mechanism

The run merger (`run_merger.hpp`) implements a three-phase approach:

1. **Detection**: Scan for ascending, descending, or constant runs at the array start
2. **Quality assessment**: Check run lengths against heuristics
3. **Decision**: If quality passes → merge runs; otherwise → fall back to quicksort

The quality heuristics control the merge/quicksort decision:

| Parameter | Java Value | Our Value | Purpose |
|-----------|------------|-----------|---------|
| MIN_FIRST_RUN_SIZE | 16 | 16 | Minimum length for first run |
| MIN_FIRST_RUNS_FACTOR | 7 | **6** | Controls minimum run length relative to array size |
| MAX_RUN_CAPACITY | 500 | 500 | Maximum runs before fallback |

**Table 3.1**: Run quality heuristic parameters.

### 3.1.4 Implementation Components

**Run Detection**: Handles ascending runs directly, reverses descending runs in-place, and treats constant runs (equal elements) as ascending. Early termination detects already-sorted arrays in O(n).

**Sequential Merge** (`merge_ops.hpp`): A two-pointer merge processes one element per comparison. The inner loop achieves good branch prediction on modern CPUs due to the predictable ternary pattern, and sequential memory access enables hardware prefetching.

**Buffer Management** (`buffer_manager.hpp`): Merge requires O(n) auxiliary space. Naive allocation (malloc per merge) adds significant overhead. The buffer manager implements pooling with 1.5× geometric growth, reducing approximately 15,000 malloc/free calls to approximately 16 allocations for a 10M element sort.

### 3.1.5 Tuning: MIN_FIRST_RUNS_FACTOR

Java's default factor of 7 was tuned for JVM characteristics. We hypothesized that C++ with `-O2 -march=native` might have different crossover points due to different memory allocation costs, function call overhead, and branch prediction behavior.

**Experiment**: Force merge vs force quicksort on 1M integers with varying run lengths.

| Run Length | Force Merge (ms) | Force Quicksort (ms) | Winner |
|------------|------------------|----------------------|--------|
| 16 | 89 | 78 | Quicksort (+14%) |
| 32 | 82 | 78 | Quicksort (+5%) |
| **64** | **76** | **78** | **Merge (-2.6%)** |
| 128 | 71 | 78 | Merge (-9%) |
| 256 | 65 | 78 | Merge (-17%) |

**Table 3.2**: Merge vs quicksort crossover analysis.

The crossover point lies between run lengths 32 and 64. Changing from factor 7 to 6 makes the quality assessment stricter, correctly rejecting short-run data that would regress under merge-based sorting.

**Validation** on representative patterns shows no regression on well-structured data while improving SAWTOOTH (short runs) by 4.4%.


---

## 3.2 Counting Sort for Small Integer Types

### 3.2.1 The Opportunity

One-byte and two-byte integral types have bounded ranges (256 or 65,536 distinct values). Instead of O(n log n) comparison-based sorting, we can achieve O(n + k) via bucket counting, where k is the range size.

### 3.2.2 Design

The implementation (`counting_sort.hpp`) follows the standard counting sort approach:
1. **Count phase**: Scan array once, incrementing frequency buckets
2. **Scatter phase**: Write elements back in sorted order

Two optimizations address the range constant k:

**Signed/Unsigned Offset**: For signed types (e.g., `int8_t` with range [-128, 127]), an offset shifts indices to non-negative values for direct array indexing.

**Sparse vs Dense Iteration**: The scatter phase must iterate over k buckets. When the array is small relative to k, most buckets are empty.

| Strategy | When Used | Approach |
|----------|-----------|----------|
| Dense | size > 128 | Iterate all buckets backward, fill from end |
| Sparse | size ≤ 128 | Skip zero buckets, fill from start |

**Table 3.4**: Counting sort iteration strategies.

### 3.2.3 Threshold Selection

Counting sort incurs fixed overhead from frequency array initialization (k operations). This must be amortized over enough elements:

| Type | Constant | Threshold | Rationale |
|------|----------|-----------|-----------|
| `uint8_t`, `int8_t` | `MIN_BYTE_COUNTING_SORT_SIZE` | 64 | Small k (256), low overhead |
| `uint16_t`, `int16_t`, `char` | `MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE` | 1,750 | Large k (65,536), need more elements |

**Table 3.5**: Counting sort thresholds by type.

Below these thresholds, the implementation falls back to comparison-based dual-pivot quicksort.

### 3.2.4 Results

For qualifying arrays, counting sort achieves O(n) performance regardless of input distribution—already-sorted, reverse-sorted, random, and duplicate-heavy inputs all complete in approximately the same time.

---

## 3.3 Floating-Point Edge Cases

### 3.3.1 IEEE-754 Challenges

Floating-point types present two challenges for sorting:

1. **NaN (Not a Number)**: By IEEE-754 definition, NaN ≠ NaN. This breaks the reflexivity assumption of comparison-based sorting—elements cannot be reliably positioned.

2. **Negative Zero**: IEEE-754 defines -0.0 and +0.0 as mathematically equal (`-0.0 == +0.0` is true), but they have distinct bit representations. A stable sort should maintain consistent ordering.

### 3.3.2 Solution

The implementation (`float_sort.hpp`) wraps the core sort with preprocessing and postprocessing phases:

**Preprocessing**:
1. Scan for NaN values using the identity `x != x` (true only for NaN)
2. Move NaNs to array end via swap
3. Convert -0.0 to +0.0 (preserving count)

**Core Sort**: Call dual-pivot quicksort on the NaN-free prefix.

**Postprocessing**:
1. Binary search to locate the zero region (values where `|x| < ε`)
2. Restore -0.0 values at the beginning of the zero region using `std::signbit()`

This approach ensures:
- NaNs are grouped at the end (following Java's convention)
- Negative zeros precede positive zeros
- All other values are correctly ordered

---

## 3.4 Chapter Summary

| Optimization | Scope | Mechanism | Impact |
|--------------|-------|-----------|--------|
| Run Merger | All types | Detect sorted runs, merge instead of partition | Up to 19× on structured data |
| Counting Sort | 1-2 byte integers | Bucket counting, O(n+k) | O(n) regardless of distribution |
| Float Preprocessing | `float`, `double` | NaN/zero handling | Correct IEEE-754 semantics |

**Table 3.6**: Summary of adaptive optimizations.

These optimizations share a common theme: **inspect the input before committing to an algorithm**. The run merger examines structure, counting sort exploits bounded ranges, and float handling addresses type-specific semantics. Together, they ensure the implementation performs well across the diverse inputs encountered in practice.

Chapter 4 extends this foundation to parallel execution, where the challenge shifts from algorithmic selection to efficient work distribution across multiple cores.
