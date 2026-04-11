# Appendix B: Complete Benchmark Data

This appendix contains representative benchmark results from the comprehensive evaluation. The full dataset (`summary_representative.csv`) includes 7,872 configurations across 6 algorithms, 4 data types, 8 patterns, and 41 array sizes.

---

## B.1 Benchmark Configuration

| Parameter | Value |
|-----------|-------|
| **Hardware** | Intel Core i5-12600KF, 20MB L3 Cache, 32GB DDR5-6000 |
| **Compiler** | g++ 13.3.0 with `-O2 -march=native -DNDEBUG` |
| **Warmup** | 3 iterations discarded |
| **Measurement** | Minimum of 30 timed iterations |
| **Array Sizes** | 41 logarithmic steps from 1K to 10M |

---

## B.2 Algorithm Key

| Algorithm ID | Description |
|-------------|-------------|
| `std_sort` | C++ Standard Library `std::sort` (Introsort baseline) |
| `dpqs_sequential` | Our implementation, single-threaded |
| `dual_pivot_parallel_2` | Our implementation, 2 threads |
| `dual_pivot_parallel_4` | Our implementation, 4 threads |
| `dual_pivot_parallel_8` | Our implementation, 8 threads |
| `dual_pivot_parallel_16` | Our implementation, 16 threads |

---

## B.3 Data Pattern Descriptions

| Pattern | Description |
|---------|-------------|
| `RANDOM` | Uniformly random integers |
| `NEARLY_SORTED` | Sorted with ~1% elements perturbed |
| `REVERSE_SORTED` | Completely descending order |
| `ORGAN_PIPE` | Ascending then descending (1,2,3,...,n/2,...,3,2,1) |
| `SAWTOOTH` | Multiple small sorted chunks |
| `MANY_DUPLICATES_10` | Only 10% unique values |
| `MANY_DUPLICATES_50` | Only 50% unique values |
| `MANY_DUPLICATES_90` | Only 90% unique values |

---

## B.4 Representative Results: int Type, RANDOM Pattern

| Size | std_sort (ms) | dpqs_seq (ms) | 16T (ms) | Speedup vs std |
|------|---------------|---------------|----------|----------------|
| 1,000 | 0.006 | 0.006 | 0.008 | 0.8× |
| 10,000 | 0.107 | 0.101 | 0.281 | 0.4× |
| 100,000 | 1.410 | 1.312 | 1.210 | 1.2× |
| 1,000,000 | 18.12 | 17.05 | 11.58 | 1.6× |
| 10,000,000 | 200.4 | 190.2 | 98.0 | 2.0× |

**Note**: Parallel overhead dominates at small sizes; benefits emerge at >100K elements.

---

## B.5 Representative Results: int Type, ORGAN_PIPE Pattern

| Size | std_sort (ms) | dpqs_seq (ms) | 16T (ms) | Speedup vs std |
|------|---------------|---------------|----------|----------------|
| 1,000 | 0.009 | 0.002 | 0.002 | 4.5× |
| 10,000 | 0.143 | 0.007 | 0.084 | 1.7× |
| 100,000 | 1.857 | 0.076 | 0.170 | **10.9×** |
| 1,000,000 | 22.09 | 0.803 | 1.330 | **16.6×** |
| 10,000,000 | 259.2 | 8.54 | 9.96 | **26.0×** |

**Key Finding**: Run merger achieves O(n) by detecting ascending+descending structure.

---

## B.6 Representative Results: int Type, REVERSE_SORTED Pattern

| Size | std_sort (ms) | dpqs_seq (ms) | 16T (ms) | Speedup vs std |
|------|---------------|---------------|----------|----------------|
| 1,000 | 0.007 | 0.001 | 0.001 | 7.0× |
| 10,000 | 0.115 | 0.004 | 0.007 | **16.4×** |
| 100,000 | 1.514 | 0.040 | 0.084 | **18.0×** |
| 1,000,000 | 17.89 | 0.416 | 0.697 | **25.7×** |
| 10,000,000 | 199.9 | 4.34 | 5.80 | **34.5×** |

**Key Finding**: Single descending run detected and reversed in O(n).

---

## B.7 Representative Results: int8_t Type (Counting Sort)

| Size | std_sort (ms) | dpqs_seq (ms) | Speedup |
|------|---------------|---------------|---------|
| 1,000 | 0.005 | 0.001 | 5.0× |
| 10,000 | 0.075 | 0.003 | **25.0×** |
| 100,000 | 1.010 | 0.024 | **42.1×** |
| 1,000,000 | 14.64 | 0.209 | **70.0×** |
| 10,000,000 | 167.4 | 2.12 | **79.0×** |

**Key Finding**: Counting sort achieves O(n) with 256-entry frequency table.

---

## B.8 Parallel Scaling Results (10M integers, RANDOM)

| Threads | Time (ms) | Speedup | Efficiency |
|---------|-----------|---------|------------|
| 1 | 508.0 | 1.00× | 100% |
| 2 | 265.0 | 1.92× | 96% |
| 4 | 154.0 | 3.30× | 82% |
| 8 | 110.0 | 4.62× | 58% |
| 16 | 98.0 | 5.18× | 32% |

**Bottleneck**: L3 cache contention (38% L3 Bound at 16T per VTune).

---

## B.9 Pattern Comparison Summary (10M int, Sequential)

| Pattern | std_sort (ms) | dpqs_seq (ms) | Speedup |
|---------|---------------|---------------|---------|
| RANDOM | 200.4 | 190.2 | 1.05× |
| NEARLY_SORTED | 82.5 | 110.3 | 0.75× |
| REVERSE_SORTED | 199.9 | 4.34 | **46.0×** |
| ORGAN_PIPE | 259.2 | 8.54 | **30.3×** |
| SAWTOOTH | 220.1 | 22.4 | **9.8×** |
| MANY_DUPLICATES_10 | 185.2 | 170.5 | 1.09× |
| MANY_DUPLICATES_50 | 175.8 | 165.2 | 1.06× |
| MANY_DUPLICATES_90 | 142.3 | 135.1 | 1.05× |

---

## B.10 CSV Data Format

The complete benchmark data is stored in CSV format with the following columns:

```csv
Algorithm,Type,Pattern,Size,Time(ms)
dual_pivot_parallel_16,double,MANY_DUPLICATES_10,1000,0.00770
dual_pivot_parallel_16,double,MANY_DUPLICATES_10,1259,0.01040
...
std_sort,int,RANDOM,10000000,200.40
```

**File locations**:
- `benchmarks/results/aggregate/summary_representative.csv` - Curated representative results
- `benchmarks/results/aggregate/summary_full.csv` - Complete raw data

---

## B.11 Data Extraction Script

```python
import pandas as pd

df = pd.read_csv('benchmarks/results/aggregate/summary_representative.csv')

# Filter for specific comparison
organ_pipe = df[
    (df['Pattern'] == 'ORGAN_PIPE') &
    (df['Type'] == 'int') &
    (df['Size'] == 10000000)
]

# Calculate speedup
std_time = organ_pipe[organ_pipe['Algorithm'] == 'std_sort']['Time(ms)'].values[0]
dpqs_time = organ_pipe[organ_pipe['Algorithm'] == 'dpqs_sequential']['Time(ms)'].values[0]
print(f"Speedup: {std_time / dpqs_time:.1f}×")
```

---

## B.12 Visualization

Interactive benchmark visualization is available via the web dashboard:

```bash
cd benchmarks
python server.py
# Open http://localhost:8000
```

The dashboard allows filtering by algorithm, type, pattern, and size range, with exportable charts.
