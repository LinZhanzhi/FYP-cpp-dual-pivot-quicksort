# Insertion Sort Variants Comparison Experiment

## Purpose
Compare different insertion sort strategies to understand their relative performance and validate design decisions.

## Variants Tested
1. **Naive**: Basic insertion sort, no optimizations
2. **Prefetch**: Simple insertion with `__builtin_prefetch` + branch hints  
3. **Pin**: Pin insertion only (separates small/large around anchor)
4. **Pair**: Pair insertion only (processes two elements at a time)
5. **Mixed**: Pin + pair combined (current implementation)

## Build & Run
```bash
cd benchmarks/insertion_comparison
g++ -std=c++17 -O2 -march=native -o tune_insertion_variants tune_insertion_variants.cpp
./tune_insertion_variants
```

## Output Files
- `insertion_comparison.csv`: Raw timing data for all variants and sizes
- `tune_insertion_variants.cpp`: Benchmark source code
- `analyze_results.py`: Analysis script (requires Python 3)
- `plot_comparison.py`: Visualization script (requires pandas, matplotlib)

## Key Findings

### 1. Prefetch Hurts Small Arrays
For arrays < 40 elements, prefetch overhead exceeds benefit (15-37% slower than naive).
Data is already in L1 cache; prefetch instruction is pure overhead.

### 2. Pair Insertion is Highly Effective
Processing two elements at a time reduces:
- Loop overhead by 50%
- Total shifts when inserting larger element first

### 3. Mixed (Pin + Pair) Optimal at Size ≥ 32
The combined strategy achieves 1.5-1.7× speedup over simple prefetch insertion.
The inner boundary of 32 (inherited from Java) aligns with experimental crossover.

### 4. Inner Boundary Not Re-Tuned for C++
The formula `end = high - 3 * ((size >> 5) << 3)` is inherited from Java.
Fine-grained tuning may yield C++-specific improvements.

## Relation to Report
This experiment supports Section 2.5.6 of the Final Report outline.
Results demonstrate evidence-based optimization decisions.
