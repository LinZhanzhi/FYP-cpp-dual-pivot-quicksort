# Report Outline Reorganization Plan

**Created**: 2026-04-09
**Purpose**: Guide for reorganizing `outline.md` from chapter-based to story-based structure
**Status**: NOT STARTED — ready to execute

---

## Problem Statement

The current outline fragments each optimization across 3 chapters:
- **Chapter 3 (Design)**: Describes the theory
- **Chapter 4 (Implementation)**: Describes the code
- **Chapter 5 (Tuning)**: Describes experiments and results

**User's critique**: "you are breaking stories apart just to conform to the design, implementation, tuning chapters. but have you think about how readers understand this? they will get lost ! and they will feel frustrated"

**Example of fragmentation**:
- Run Merger design → Section 3.5
- Run Merger implementation → Section 4.1.4
- Run Merger tuning → Section 5.5 (this has ~100 lines of detailed expansion)
- Reader must jump between 3 locations to understand ONE feature

---

## Proposed Solution: Story-Based Structure

Each optimization becomes ONE complete narrative:
```
Problem → Design → Implementation → Tuning → Final Result
```

Reader follows the full story without jumping chapters.

---

## Current Structure (outline.md)

```
Chapter 1: Introduction (4-5 pages) — KEEP AS IS
Chapter 2: Literature Review (6-8 pages) — KEEP AS IS
Chapter 3: Design and Methodology (8-10 pages) — REORGANIZE
Chapter 4: Implementation (8-10 pages) — REORGANIZE
Chapter 5: Algorithm Engineering and Tuning (6-8 pages) — REORGANIZE
Chapter 6: Results and Evaluation (10-12 pages) — KEEP AS IS
Chapter 7: Discussion (4-5 pages) — KEEP AS IS
Chapter 8: Conclusion (2-3 pages) — KEEP AS IS
```

---

## Proposed New Structure

```
Chapter 1: Introduction (4-5 pages) — UNCHANGED
Chapter 2: Literature Review (6-8 pages) — UNCHANGED

Chapter 3: Core Algorithm — Dual-Pivot Quicksort (6-8 pages)
  └── Complete story: partitioning + pivot selection + insertion sort base case

Chapter 4: Adaptive Optimizations (10-12 pages)
  └── Story 1: Insertion Sort Threshold
  └── Story 2: Run Merger (the hero feature — 19× speedup)
  └── Story 3: Type-Specific Paths (counting sort, float handling)

Chapter 5: Parallel Execution (10-12 pages)
  └── Story: Thread Pool Evolution (V1 → V2 → V3)
  └── Story: Parallel Merge and Granularity
  └── Story: Memory Wall and Scaling Ceiling (VTune analysis)

Chapter 6: Results and Evaluation (10-12 pages) — KEEP CURRENT CONTENT
Chapter 7: Discussion (4-5 pages) — KEEP CURRENT CONTENT
Chapter 8: Conclusion (2-3 pages) — KEEP CURRENT CONTENT
```

---

## Detailed Mapping: What Goes Where

### NEW Chapter 3: Core Algorithm — Dual-Pivot Quicksort

**Sources to merge**:
| Current Location | Content | New Section |
|------------------|---------|-------------|
| 3.3 Core Algorithm Design | Three-way partitioning, pivot selection | 3.1 The Algorithm |
| 4.1.1 partition.hpp | Backward scanning, Dutch National Flag | 3.2 Partitioning Implementation |
| 4.1.2 sequential_sorters.hpp | Sorting network, equidistant sampling | 3.3 Pivot Selection |
| 4.1.3 insertion_sort.hpp | Simple/mixed insertion, prefetching | 3.4 Small Array Base Case |
| 5.3 Insertion Sort Threshold | Threshold tuning methodology | 3.5 Threshold Tuning (insert here) |
| 4.3.1 Tail Call Optimization | Stack overflow prevention | 3.6 Recursion Safety |

**Structure**:
```
3.1 The Core Algorithm
  - Yaroslavskiy's three-way partitioning
  - Why two pivots? (Wild's analysis)

3.2 Partitioning Implementation
  - [< P1] [P1 ≤ x ≤ P2] [> P2] invariant
  - Backward scanning for cache efficiency
  - Dutch National Flag fallback

3.3 Pivot Selection
  - Optimal 9-comparator sorting network
  - Equidistant sampling positions

3.4 Small Array Optimization (Complete Story)
  - Problem: Recursion overhead dominates at small sizes
  - Design: Insertion sort cutoff
  - Implementation: Simple vs mixed insertion
  - Tuning: INSERTION_SORT_THRESHOLD sweep (from 5.3)
  - Result: 54 optimal for random, conservative 44 chosen

3.5 Recursion Safety
  - Tail call optimization
  - Heapsort fallback at MAX_RECURSION_DEPTH
```

---

### NEW Chapter 4: Adaptive Optimizations

**Story 1: Run Merger (the hero feature)**

**Sources to merge**:
| Current Location | Content |
|------------------|---------|
| 3.5 Adaptive Algorithm Selection | Quality heuristics design |
| 4.1.4 run_merger.hpp | Run detection, merge tree |
| **5.5 Run Merger Heuristic Tuning** | **EXTENSIVE CONTENT — ~100 lines** |

**⚠️ CRITICAL**: Section 5.5 has been expanded with:
- Background explanation
- Code snippet showing `MIN_FIRST_RUNS_FACTOR`
- Results tables with median runtimes
- Regression warnings
- Lesson Learned block

**Structure**:
```
4.1 Run Merger: Exploiting Sorted Runs
  4.1.1 The Problem
    - Many real-world datasets have pre-existing order
    - std::sort ignores this → does full O(n log n) work

  4.1.2 Design: Timsort-Inspired Run Detection
    - Ascending, descending, constant run handling
    - Quality heuristics: MIN_FIRST_RUNS_FACTOR, MAX_RUN_CAPACITY

  4.1.3 Implementation (run_merger.hpp)
    - Run detection algorithm
    - Merge tree construction
    - Already-sorted early termination

  4.1.4 Tuning Experiments (PRESERVE ALL FROM 5.5)
    [COPY ENTIRE SECTION 5.5 HERE]
    - Background
    - Code snippet
    - Results table
    - Analysis
    - Lesson Learned

  4.1.5 Result
    - 19× speedup on ORGAN_PIPE
    - 6× on REVERSE_SORTED
    - O(n) vs O(n log n)
```

**Story 2: Type-Specific Paths**

**Sources to merge**:
| Current Location | Content |
|------------------|---------|
| 3.4.1 Counting Sort | O(n) for byte/short |
| 3.4.2 Floating-Point Handling | NaN, negative zero |
| 5.4 Counting Sort | Tuning considerations |

**Structure**:
```
4.2 Counting Sort for Small Integer Types
  4.2.1 The Opportunity
    - 1-byte and 2-byte types have bounded range
    - Can achieve O(n) via bucket counting

  4.2.2 Implementation
    - Signed/unsigned offset calculation
    - Sparse vs Dense optimization

  4.2.3 Result
    - [Add benchmark data if available]

4.3 Floating-Point Edge Cases
  4.3.1 IEEE-754 Challenges
    - NaN ≠ NaN breaks comparison
    - -0.0 == +0.0 mathematically but need ordering

  4.3.2 Solution
    - Preprocessing: Move NaNs, convert zeros
    - Postprocessing: Restore -0.0 positions
```

---

### NEW Chapter 5: Parallel Execution

**Story 1: Thread Pool Evolution**

**Sources to merge**:
| Current Location | Content |
|------------------|---------|
| 3.6 Parallel Architecture | Work-stealing design |
| 4.2 Parallel Implementation Evolution | V1 → V2 → V3 progression |
| 4.2.1-4.2.3 Phase 1-3 | Adaptive granularity, sticky victim, depth cutoff |
| 4.3.2-4.3.4 | ThreadPool quiescence, ForkJoinTask, type erasure |

**Structure**:
```
5.1 Building a Work-Stealing Thread Pool
  5.1.1 Why Work-Stealing?
    - Recursive algorithms create imbalanced work
    - Static scheduling leads to idle threads

  5.1.2 Evolution: Three Generations
    Phase V1: Single Global Mutex (baseline)
    - Simple but contention-heavy

    Phase V2: Per-Thread Queues with Central Dispatch
    - Better but still bottlenecked

    Phase V3: Work-Stealing with LIFO/FIFO (final)
    - LIFO local access for cache locality
    - FIFO stealing for load balance

  5.1.3 Implementation Details
    - WorkStealingQueue per thread
    - try_lock for non-blocking steal
    - CountedCompleter pattern from Java ForkJoinPool

  5.1.4 Optimizations Applied
    - Adaptive granularity (Phase 1)
    - Sticky victim for cache locality (Phase 2)
    - Depth cutoff hybrid (Phase 3)

  5.1.5 Result
    - Broke 4.4× plateau → achieved 5.18× speedup
```

**Story 2: Parallel Merge**

**Sources to merge**:
| Current Location | Content |
|------------------|---------|
| 3.6.3 Parallel Merge | Binary search partitioning |
| **5.6 Parallel Merge Threshold** | **EXTENSIVE CONTENT — ~80 lines** |

**⚠️ CRITICAL**: Section 5.6 has been expanded with:
- Parameter description with code snippet
- Hypothesis for too small/too large
- Methodology table
- Results table (128-65536 threshold sweep)
- "Surprising Finding" analysis
- Why 4096 was retained table
- Lesson Learned block

**Structure**:
```
5.2 Parallel Merge Operations
  5.2.1 The Problem
    - Merge is sequential by nature
    - Need to parallelize for structured data

  5.2.2 Design: Binary Search Split
    - Recursive subdivision until threshold
    - Fork two independent merge tasks

  5.2.3 Tuning (PRESERVE ALL FROM 5.6)
    [COPY ENTIRE SECTION 5.6 HERE]
    - Parameter description
    - Hypothesis
    - Methodology
    - Results
    - Surprising Finding
    - Why 4096 was retained
    - Lesson Learned
```

**Story 3: Memory Wall and Scaling Ceiling**

**Sources to merge**:
| Current Location | Content |
|------------------|---------|
| 6.3 Parallel Scaling Analysis | VTune profiling data |
| 6.3.1 Speedup Results | CPI degradation table |
| 6.3.2 VTune Bottleneck Analysis | L3 cache contention |
| 6.3.3 Amdahl's Law Application | Serial fraction estimation |
| 6.3.4 VTune-Guided Optimizations | What worked, what didn't |

**Note**: This content is currently in Chapter 6. Decision:
- Option A: Keep in Chapter 6 (Results) — it's evaluation data
- Option B: Move to Chapter 5 — it's about parallel execution

**Recommendation**: Keep in Chapter 6 since it's empirical results, but reference from Chapter 5.

---

### NEW Chapter 5 Additional Content: Negative Results

**Sources to merge**:
| Current Location | Content |
|------------------|---------|
| **5.7 Negative Results** | **EXTENSIVE CONTENT — ~150 lines** |
| 5.7.1 SBO Analysis | Small Buffer Optimization experiment |
| 5.7.2 Explicit Memory Management | Arena/pool allocators |
| 5.7.3 Sequential vs Parallel (1 Thread) | Single-threaded overhead |

**⚠️ CRITICAL**: Section 5.7 has been extensively expanded with:
- Full code snippets
- Theoretical analysis tables
- Measurement results
- Root cause analysis
- Lesson Learned blocks

**Structure**:
```
5.3 What Didn't Work (Negative Results)
  5.3.1 Small Buffer Optimization (PRESERVE ALL FROM 5.7.1)
    [COPY ENTIRE SECTION 5.7.1 HERE]

  5.3.2 Custom Memory Allocators (PRESERVE ALL FROM 5.7.2)
    [COPY ENTIRE SECTION 5.7.2 HERE]

  5.3.3 Single-Threaded Parallel Path (PRESERVE ALL FROM 5.7.3)
    [COPY ENTIRE SECTION 5.7.3 HERE]
```

---

### Content to Remove/Already Removed

These sections were removed in previous cleanup:
- ~~Section 3.8 Benchmarking Infrastructure~~ — organizational trivia
- ~~Section 4.3.1 Race Condition~~ — expected fix, not noteworthy
- ~~Section about Mutex Contention~~ — already shown positively in evolution

---

## Execution Checklist

```markdown
- [ ] Create backup of current outline.md
- [ ] Restructure Chapter 3: Core Algorithm
  - [ ] Merge sections 3.3, 4.1.1-4.1.3, 5.3, 4.3.1
  - [ ] Write connecting narrative
- [ ] Restructure Chapter 4: Adaptive Optimizations
  - [ ] Create Run Merger complete story
  - [ ] PRESERVE all content from Section 5.5
  - [ ] Create Type-Specific Paths story
- [ ] Restructure Chapter 5: Parallel Execution
  - [ ] Create Thread Pool Evolution story
  - [ ] Create Parallel Merge story
  - [ ] PRESERVE all content from Section 5.6
  - [ ] Add Negative Results section
  - [ ] PRESERVE all content from Sections 5.7.1-5.7.3
- [ ] Update Chapter 6 to reference new locations
- [ ] Update Chapter 7 Discussion if references change
- [ ] Verify no content was lost
- [ ] Update page budget estimates
```

---

## Content to PRESERVE (Do Not Lose!)

### Section 5.5 Run Merger Heuristic Tuning (Lines ~283-376)
- Background explanation
- Code snippet: `for (int k = left; ++k < right && comp(a[k - 1], a[k]); );`
- Simplified pseudocode
- Results table (MEDIAN runtime for MIN_FIRST_RUNS_FACTOR values)
- Regression warning paragraph
- Lesson Learned block

### Section 5.6 Parallel Merge Threshold (Lines ~450-550)
- Parameter description with code snippet `parallel_merge()`
- Hypothesis table (too small vs too large)
- Methodology table (threshold vs tasks created)
- Results table (128-65536 sweep)
- "Surprising Finding" analysis (3% spread across 512× range)
- Why Performance is Insensitive analysis (4 numbered points)
- Why 4096 Was Retained comparison table
- Decision Rationale
- Lesson Learned block
- Design Decision statement

### Section 5.7.1 SBO Analysis (Lines ~550-700)
- Hypothesis explaining std::function overhead
- Background: What is SBO code snippet
- Three attempted approaches:
  1. Custom Task Wrapper code
  2. Ring Buffer Task Queue code
  3. Inlined Invocation
- Theoretical Overhead Comparison table
- Measurement Results table
- "Why It Failed" analysis (3 numbered points)
- Code Archaeology reference
- Lesson Learned block
- Design Decision statement

### Section 5.7.2 Explicit Memory Management (Lines ~700-780)
- Hypothesis about allocation overhead
- Theoretical Analysis table
- Four attempted approaches
- "Why It Failed" section with comparison table
- Measurement Results
- Root Cause Analysis
- Lesson Learned block
- Design Decision statement

### Section 5.7.3 Sequential vs Parallel (1 Thread) (Lines ~780-850)
- Hypothesis about LIFO equivalence
- Theoretical Analysis comparison table
- Overhead Sources list
- FIGURE placeholder with reproduction commands
- Findings table
- Conclusion statement
- Design Decision statement

### Section 5.8 Compiler Optimization Flag Tuning (Lines ~850-930)
- Methodology (12 flag combinations)
- Results Summary table
- Key Findings (4 numbered points)
- Final Configuration code block
- Rationale statement

---

## Page Budget (Updated Estimate)

| Chapter | Current Pages | Proposed Pages |
|---------|---------------|----------------|
| Chapter 1: Introduction | 5 | 5 |
| Chapter 2: Literature Review | 7 | 7 |
| Chapter 3: Core Algorithm | — | 7 |
| Chapter 4: Adaptive Optimizations | — | 11 |
| Chapter 5: Parallel Execution | — | 10 |
| Chapter 6: Results and Evaluation | 11 | 10 |
| Chapter 7: Discussion | 4 | 4 |
| Chapter 8: Conclusion | 3 | 3 |
| **Total** | ~45 | **~47** |

---

## Files to Reference

- Current outline: `PaperWork/FinalReport/outline.md`
- VTune analysis: `report/vtune_bottleneck_analysis.md`
- Space optimization: `docs/space_optimization_report.md`
- Scaling analysis: `docs/scaling_analysis_v3.md`

---

## How to Execute This Plan

1. Open `outline.md` in editor
2. Create new file `outline_v2.md` as working copy
3. Copy Chapters 1-2 unchanged
4. Build Chapter 3 by merging content per mapping above
5. Build Chapter 4 (critical: preserve all 5.5 content)
6. Build Chapter 5 (critical: preserve all 5.6, 5.7.x content)
7. Copy Chapters 6-8, adjusting internal references
8. Diff `outline.md` vs `outline_v2.md` to verify no content lost
9. Replace `outline.md` with `outline_v2.md`
10. Git commit with message: "Reorganize outline: story-based structure"

---

**END OF PLAN**
