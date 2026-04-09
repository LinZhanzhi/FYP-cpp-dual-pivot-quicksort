# Tuning Experiments

This directory contains all constant tuning experiments for the Dual-Pivot Quicksort implementation.

## Directory Structure

| Directory | Parameter Tuned | Optimal Value |
|-----------|-----------------|---------------|
| `insertion-sort-threshold/` | `MAX_INSERTION_SORT_SIZE` | 60 |
| `counting-sort/` | `MIN_BYTE_COUNTING_SORT_SIZE`, `MIN_SHORT_COUNTING_SORT_SIZE` | 64, 1750 |
| `merge-threshold/` | `MIN_PARALLEL_MERGE_PARTS_SIZE` | 4096 |
| `mixed-insertion-sort/` | `MAX_MIXED_INSERTION_SORT_SIZE` | 65 |
| `first-runs/` | `MIN_FIRST_RUNS_FACTOR` | 6 |
| `sbo-experiment/` | Small Buffer Optimization | N/A (negative result) |
| `vtune-profiling/` | Intel VTune hotspot analysis | - |

## Shared Infrastructure

- `shared/tune_constants.py` - Compile-time parameter injection framework
- `shared/run_tuning_driver.py` - General tuning driver script

## Results and Logs

- `results/` - Aggregated tuning results
- `logs/` - Tuning run logs
