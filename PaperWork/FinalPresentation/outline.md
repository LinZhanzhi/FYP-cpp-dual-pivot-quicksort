# Final Presentation Outline
**Topic:** High-Performance C++ Dual-Pivot Quicksort — A Drop-in Replacement for `std::sort`
**Duration:** 20 minutes (including Q&A)

> **Framing:** Unlike the Final Report (which narrates the *journey* of solving problems chapter by chapter), this presentation is **product-focused**. We walk the audience through what happens inside `dpqs::sort(...)` from the moment it is called, in the **exact order of execution**. Every design decision is justified at the point where the code actually encounters it.

---

## Time Breakdown

| # | Section | Duration | Purpose |
|---|---------|----------|---------|
| 1 | Introduction & Product Pitch | 2 min | What is DPQS and why use it |
| 1.5 | Research Gap + Methodology | 1.5 min | Position the work; state how every number is measured |
| 2 | **Sequential Walkthrough + Per-Feature Results** | 8 min | 🔥 Each guard shown with the input pattern it accelerates |
| 3 | **Parallel Execution + Scaling Results** | 3.5 min | Thread-count scaling; motivates §4 |
| 4 | **VTune Root-Cause: L3 Contention** | 3 min | Why scaling plateaus |
| 5 | Engineering, Limitations, Reflection & Conclusion | 2 min | C++ craft, honest limits, takeaway |

---

## 1. Introduction & Product Pitch (2 min)

- **The product:** `dpqs::sort(...)` — a header-only, C++17, STL-compatible sorting library.
- **The pitch (one slide):**
  - Drop-in replacement for `std::sort` — same iterator interface, same comparator support.
  - Built on **Yaroslavskiy's dual-pivot quicksort** — the family of algorithms proven in production on billions of devices.
  - Adds: adaptive run-merging, counting-sort specialization, float/NaN handling, and a work-stealing parallel mode.
  - **Delivers** parity with `std::sort` on random data, **up to 19× faster** on structured data, and **4.72× parallel speedup** (peak, at 8 threads on a hybrid-core CPU — §3 + §4 explain the shape of the curve).
- **How to call it** (show this slide early so the audience has a mental anchor):
  ```cpp
  std::vector<int> v = {...};
  dual_pivot::sort(v);                          // auto-parallel
  dual_pivot::sort(v.begin(), v.end());         // iterator form
  dual_pivot::sort(v, 1);                       // force sequential
  dual_pivot::sort(v, std::greater<int>{});     // custom comparator
  ```

---

## 1.5 Research Gap + Methodology (1.5 min)

### Research gap (≈ 30 s, 1 slide)
- `std::sort` (libstdc++) — introsort; no adaptive run-merging, no duplicate-aware partition.
- `pdqsort` (Boost / libc++) — pattern-defeating single-pivot; excellent on random, no parallelism.
- `ips4o`, `parallel_stable_sort` — parallel, but not header-only and not drop-in via the familiar `std::sort` call shape.
- **Gap this FYP fills:** a single header that combines **dual-pivot partitioning + Timsort-style run-merging + duplicate-aware DNF + work-stealing parallelism**, behind the exact `std::sort` API.

### Methodology (≈ 60 s, 1 slide — trust-building)
- **Hardware:** [CPU model, cores/threads, L3 size — fill in from `get_system_info.py`]. L3 size matters for §4.
- **Compiler:** GCC 13.3.0, `-O2 -march=native -DNDEBUG`, C++17, pthreads.
- **Workload:** 10⁷ elements (and a 41-point size sweep from 10³ to 10⁷ in the full harness, 7 800+ configurations total).
- **Randomness control — industry-standard multi-seed protocol:**
  - 10 distinct seeds (42…51), regenerated input per seed.
  - Per seed: 3 warmups + 10 timed iterations.
  - Representative = **median-of-per-seed-medians** (100 samples total).
  - Structured patterns (nearly-sorted, reverse, duplicates, organ-pipe, sawtooth) fixed-seed, 30 iterations, min.
- **Outlier handling:** medians absorb single-run jitter; median-of-medians absorbs one unlucky seed.

> This slide buys trust for every number on every later slide.

---

## 2. Sequential Walkthrough — Each Guard, and the Results It Buys Us (8 min) 🔥

> This is the spine of the presentation. We follow **one call** through the decision tree in execution order, and **immediately after introducing each guard we show the benchmark number it is responsible for**. The audience sees cause (code) and effect (number) back-to-back. Prior-art attribution is mentioned briefly *inline* as each technique appears — no separate literature dump.

### Step 0 — Entry & Normalization
- User calls `sort(container)` / `sort(iter, iter)` / `sort(ptr, length)`.
- Convenience wrappers normalize to `sort(T* a, int parallelism, ptrdiff_t low, ptrdiff_t high, Compare comp)`.
- Iterator adapter: contiguous iterators unwrapped to raw pointers; non-contiguous iterators go via a temporary buffer.
- Default parallelism = `std::thread::hardware_concurrency()`.

### Step 1 — Guards & Early Termination
- `low >= high` → return immediately.
- `checkNotNull(a)` and range validation → throw on misuse.
- **`checkEarlyTermination`:** single linear scan; returns in **O(n)** if already sorted.

📊 **Result — fully-sorted input (10⁷ int):**

| Algorithm | Runtime |
|-----------|---------|
| `std::sort` | ~baseline |
| `dpqs` | **near-instant (O(n) scan)** |

*Takeaway: the cheapest guard wins the easiest case outright.*

### Step 2 — Type-Based Dispatch (compile-time `if constexpr`)

| Condition | Strategy | Why |
|-----------|----------|-----|
| `T` integral AND `sizeof(T) ≤ 2` | **Counting sort** (classical — Seward, 1954) | O(n) with a 256 / 65 536-bucket histogram |
| `T` floating-point (sequential) | **`sort_floats`** | NaN / ±0.0 handled correctly |
| `parallelism > 1` AND `size > MIN_PARALLEL_SORT_SIZE` | **Parallel path** (§3) | Amortizes thread overhead |
| Otherwise | **Sequential dual-pivot** | General case |

📊 **Result — small-integer types (10⁷ elements, random):**

| Type | `std::sort` | `dpqs` | Speedup |
|------|-------------|--------|---------|
| `int8_t` | baseline | counting sort | **~8–10×** |
| `int16_t` | baseline | counting sort | **~5–7×** |

![int8_t sequential vs std::sort](image/int8_t%20sequential%20dpqs%20vs%20stdsort.png)
![int16_t sequential vs std::sort](image/int16_t%20sequential%20vs%20stdsort.png)

*Takeaway: picking the right algorithm before sorting even starts pays off the most.*

### Step 3 — Adaptive Recursion Budget (Introsort safety net, Musser 1997)
- `max_depth = 2 · ⌊log₂ n⌋ · DELTA` computed once before recursion.
- Crosses budget → heapsort → O(n log n) worst case. (`std::sort` uses the same trick.)
- No standalone benchmark number — this is the safety net that makes every other number trustworthy on adversarial inputs.

### Step 4 — The Recursive Core: `sort_sequential(...)`
At each call the algorithm evaluates the following guards **in order** and stops at the first one that applies.

#### 4.1 / 4.2 Small partitions → **insertion sort leaves** (classical)
- `size < 65` non-leftmost → mixed insertion (pin-insertion; left pivot acts as sentinel so no lower-bound check).
- `size < 45` leftmost → plain insertion.
- This is the same small-partition cutoff idea used by `std::sort` and every mature quicksort; well-chosen thresholds matter more than the variant.

📊 **Combined contribution — visible in the random-data result:**

| Pattern | `std::sort` | `dpqs` | Speedup |
|---------|-------------|--------|---------|
| Random `int` (10⁷) | baseline | ~parity | **~1.0×** |

![Random int32 runtime: dpqs vs std::sort](image/Runtime%20comparison%20of%20sequential%20Dual-Pivot%20Quicksort%20and%20stdsort%20on%20random%2032-bit%20integers.png)

*Takeaway: without these insertion-sort leaves we would lose on random data. They are what keeps us at parity with a very mature `std::sort`.*

#### 4.3 `try_merge_runs` — adaptive run-merging (Timsort lineage, Peters 2002)
Scans forward for ascending / descending / constant runs. If the first run is shorter than `MIN_FIRST_RUN_SIZE` it bails out immediately — a short comparison loop, no allocation — so random data pays only a tiny fail-fast cost. On structured input it finds long runs and merges them in O(n log r), returning without ever partitioning.
*Novelty point:* combining Timsort-style run detection **in front of** a dual-pivot quicksort (rather than as a standalone algorithm) is what gives us both the 19× nearly-sorted win *and* the near-zero random-data penalty.

📊 **Result — structured patterns (10⁷ int):**

| Pattern | `std::sort` | `dpqs` | Speedup |
|---------|-------------|--------|---------|
| Nearly-sorted | baseline | run-merge path | **up to 19×** |
| Reverse-sorted | baseline | detected + flipped | **~2×** |
| Organ-pipe | baseline | run-merge / DNF | *(see figure)* |
| Sawtooth | baseline | run-merge | *(see figure)* |

![Nearly-sorted runtime](image/%5Bint%5D%5BNearlySorted%5D%5BRuntime%5D%20dpqs_sequential%20vs%20stdsort.png)
![Reverse-sorted runtime](image/%5Bint%5D%5BReverseSorted%5D%5BRuntime%5D%20dpqs_sequential%20vs%20stdsort.png)
![Organ-pipe runtime](image/%5Bint%5D%5BOrganPipe%5D%5BRuntime%5D%20dpqs_sequential%20vs%20stdsort.png)
![Sawtooth runtime](image/%5Bint%5D%5BSawTooth%5D%5BRuntime%5D%20dpqs_sequential%20vs%20stdsort.png)

*Takeaway: one scan + one merge pass turns a full sort into linearithmic work in `r` (number of runs).*

#### 4.4 Depth budget exceeded → **heapsort fallback**
Worst-case guarantee. No headline number (healthy inputs never hit it); its value is that adversarial inputs can't push us into O(n²).

#### 4.5 Pivot sampling — `sort5_network`
- Pick 5 indices `e1..e5` spread evenly (step = `(size/8)·3 + 3`).
- Sort them with a hardcoded 9-comparator optimal sorting network → order statistics for free.
- Richer than the median-of-3 used by traditional quicksort; the cost is amortized because 5 sorted samples give us **both pivots and a duplicate oracle** (see 4.6).

#### 4.6 Strict ordering test `a[e1] < a[e2] < a[e3] < a[e4] < a[e5]` → branch between two partitioners
- **True → dual-pivot partition** (Yaroslavskiy 2009) with `P1 = a[e1]`, `P2 = a[e5]`. Three regions `[< P1] [P1 ≤ x ≤ P2] [> P2]`. Cache-friendly backward scan + `DPQS_PREFETCH_READ` hints.
- **False (duplicates detected) → single-pivot Dutch National Flag partition** (Dijkstra, 1976) with `P = a[e3]`. Produces `[< P] [= P] [> P]`; middle region excluded from recursion.

📊 **Result — duplicate-heavy patterns (10⁷ int):**

| Pattern | `std::sort` | `dpqs` | Speedup |
|---------|-------------|--------|---------|
| 10% duplicates | baseline | dual-pivot | ~1.1× |
| 50% duplicates | baseline | DNF kicks in | ~1.5× |
| 90% duplicates | baseline | DNF dominates | **2–3×** |

![10% duplicates runtime](image/%5Bint%5D%5BManyDuplicate10%5D%5BRuntime%5D%20dpqs_sequential%20vs%20stdsort.png)
![50% duplicates runtime](image/%5Bint%5D%5BManyDuplicate50%5D%5BRuntime%5D%20dpqs_sequential%20vs%20stdsort.png)
![90% duplicates runtime](image/%5Bint%5D%5BManyDuplicate90%5D%5BRuntime%5D%20dpqs_sequential%20vs%20stdsort.png)

*Takeaway: the 5-sample ordering test is a free oracle — on duplicate-heavy data it routes us to DNF automatically.*

#### 4.7 Tail-call optimized recursion
Always **loop** on the largest sub-partition, **recurse** on the two smaller ones → stack stays O(log n).

### Step 5 — Return
Recursion unwinds; array is sorted in place.

📊 **Section 2 summary slide — sequential `dpqs` vs `std::sort` (10⁷ int):**

| Pattern | Speedup | Responsible guard |
|---------|---------|-------------------|
| Already sorted | near-instant | early termination (Step 1) |
| `int8_t` / `int16_t` random | ~8× / ~6× | counting-sort dispatch (Step 2) |
| Random `int` | ~1.0× | insertion-sort leaves (4.1, 4.2) |
| Nearly-sorted | up to 19× | `try_merge_runs` (4.3) |
| Reverse-sorted | ~2× | `try_merge_runs` (4.3) |
| 90% duplicates | 2–3× | DNF partition (4.6) |

---

## 3. Parallel Execution — Scaling Across Threads (3.5 min)

When Step 2 routes to `parallelQuickSort`, the **same Step-4 recursion runs** but with a `Sorter` attached:

- **Thread pool** with per-worker double-ended queues — a **work-stealing** scheduler in the Blumofe–Leiserson / Chase–Lev lineage (current implementation uses per-deque mutexes; a lock-free version is listed in future work).
- **Load-balancing rule:** at each partition, fork the two larger sub-ranges as tasks, keep the smallest for the current worker (loop iteration).
- **Work stealing:** idle workers steal from the tail of victim deques; sticky-victim heuristic reduces contention.
- **`MIN_PARALLEL_SORT_SIZE` cutoff** prevents spawning tasks dominated by sync cost.

📊 **Result — 10⁷ random `int`, multi-seed median-of-medians (10 seeds × 10 iters):**

| Threads | Runtime (ms) | Speedup vs 1T | Efficiency | Regime |
|---------|--------------|---------------|------------|--------|
| 1 (sequential) | 513.4 | 1.00× | 100 % | — |
| 2 | 394.7 | 1.30× | 65 % | Fork/merge overhead still visible |
| 4 | 159.8 | **3.21×** | 80 % | 4 P-cores engaged, caches private |
| 8 | **108.7** | **4.72×** | 59 % | Sweet spot — all 6 P-cores + 2 E-cores |
| 16 | 244.9 | 2.10× | 13 % | **Regression** — see §4 |

![Parallel scaling: std::sort vs dpqs at 1/2/4/8/16 threads](image/Runtime%20on%20random%20integers%20for%20stdsort%20and%20Dual-Pivot%20Quicksort%20with%201%2C%202%2C%204%2C%208%2C%20and%2016%20threads..png)

*Observation:* scaling **peaks at 8 threads** (4.72×), then **reverses** — 16 threads is 2.25× slower than 8 threads. This is not a plateau; it is a regression. Something about going from 8 → 16 threads actively hurts. §4 uses VTune to attribute it to four stacked hardware effects.

*Slide:* partition-tree diagram + the scaling chart with the 8T peak circled.

> **Placeholder needed:** work-stealing diagram (not yet in `image/`).

---

## 4. VTune Root-Cause Analysis — Four Stacked Ceilings Explain the 8→16 Regression (3 min)

**Hardware under test:** Intel Core i5-12600KF (Alder Lake hybrid): **6 P-cores (SMT, 12 logical threads) + 4 E-cores (no SMT, 4 logical threads) = 10 physical / 16 logical**. L3 shared across all cores = 20 MB. This topology is essential context.

**Method:** Intel VTune `uarch-exploration` (top-down microarchitecture analysis) on the same workload at **8 threads** and **16 threads**, comparing the counter deltas.

📊 **Key counter deltas (P-core top-down breakdown):**

| Metric | 8T | 16T | Δ | What it tells us |
|--------|----|----|----|------------------|
| **L3-Bound** (% clockticks) | 14.5 % | **25.1 %** | **+10.6 pp** | Biggest single shift — aggregate working set no longer fits in the shared 20 MB L3 |
| **Machine Clears** (% slots) | 1.8 % | **7.5 %** | **×4.2** | Memory-ordering nukes — classic SMT signature when two threads share a core's store buffer |
| L2-Bound (% clockticks) | 0.0 % | 1.0 % | +1.0 pp | SMT siblings now share the P-core's L2 |
| L1-Bound (% clockticks) | 13.8 % | 14.7 % | +0.9 pp | SMT siblings now share the P-core's L1D |
| Slow-Pause spin-wait | 0.0 % | 0.8 % | +0.8 pp | Thread-pool mutex / steal contention when 16 workers thrash the deques |
| E-core clockticks | 10.9 G | **29.8 G** | **×2.7** | E-cores are actively recruited (they weren't at 8T) |
| E-core CPI | 1.37 | 1.37 | — | E-cores are ~20 % slower per instruction than P-cores (1.14) |
| **P-core overall CPI** | 1.14 | **1.55** | **+36 %** | Every retired instruction now pays more cycles — sum of the above |
| DRAM-Bound | 0.6 % | 0.5 % | ~0 | **Bandwidth is not the bottleneck — latency is** |

**Four stacked ceilings (in order of impact):**

1. **L3 latency contention.** Biggest counter shift. At 8T each worker's partition comfortably shares the 20 MB L3; at 16T the collective footprint blows past it and every miss pays DRAM round-trip latency. DRAM *bandwidth* is fine; DRAM *latency* is the killer.
2. **Hybrid-core dilution.** 16T forces work onto the 4 E-cores, which retire ~20 % fewer instructions per cycle than P-cores and have their own shared L2 cluster. More wall-clock is burned on the slower cores.
3. **SMT L1/L2 sharing.** Past 12 logical threads, P-cores run two sorts simultaneously. The L1D (48 KB) and L2 (1.25 MB) that were private at 8T are now halved. Machine-clear rate *quadruples* — classic store-buffer / memory-order pressure from SMT siblings touching independent memory.
4. **Thread-pool mutex contention.** Slow-Pause appears (0 → 0.8 %). With 16 workers, end-of-sort steal attempts pile up on the per-deque mutex; a lock-free Chase–Lev deque would reduce this — listed in future work.

**Takeaway:** the 8 → 16 regression is **not** algorithmic, **not** a scheduling bug, and **not** a bandwidth wall. It is the sum of four hardware-level ceilings that start biting simultaneously when we cross the 12-logical-thread boundary on a hybrid-core CPU. The algorithm and the thread pool are doing their jobs — the silicon is the bottleneck.

*Slide:* single side-by-side table (8T vs 16T VTune deltas) + the four-bullet interpretation. Annotate the §3 scaling chart with the 8T peak.

> **Placeholder needed:** VTune top-down screenshot (optional — the counter table conveys the same content).

---

## 5. Engineering, Limitations, Reflection & Conclusion (2 min)

### 5.1 Engineering behind the API — "Using C++ well" (≈ 40 s, 1 slide)
The algorithm chapter is over; this slide shows the language-level work that makes the library *usable*.

- **Header-only, zero dependencies** — single `#include`, drops into any C++17 build.
- **Fully generic via templates** — `T`, iterator category, and comparator are all template parameters; works for any type with `operator<` or any supplied `Compare`.
- **STL-compatible iterator adapter** — contiguous iterators are unwrapped to raw pointers (zero-cost fast path); non-contiguous iterators fall back to a temporary buffer. Call-shape identical to `std::sort`.
- **Compile-time dispatch via `if constexpr`** — the counting-sort / float / dual-pivot decision is resolved at compile time; no runtime branching in hot paths.
- **Hardware-aware hints** — `DPQS_PREFETCH_READ` macros wrap `__builtin_prefetch` for the backward-scan partition loop.
- **Header organization** — `dpqs/` split into focused modules (`sequential_sorters.hpp`, `run_merger.hpp`, `counting_sort.hpp`, `thread_pool.hpp`, `utils.hpp`) so each optimization is reviewable in isolation.

### 5.2 Honest Limitations (≈ 30 s, part of the conclusion slide)
- **Not stable.** Equal elements may be reordered — like `std::sort`, unlike `std::stable_sort`.
- **Not a drop-in for `std::stable_sort`** for the same reason; users who rely on stability must stay with the STL version.
- **Move-only types.** The run-merger path currently assumes copyable elements (scratch buffer); move-only `T` is a known gap to close.
- **Regression on certain nearly-sorted sizes** — run-merger threshold tuning is incomplete.
- **No SIMD partitioning** — BlockQuicksort / vectorized partition is a natural next step.
- **Mutex-guarded deques** — a lock-free Chase–Lev deque should push scaling further before the L3 wall dominates.

### 5.3 Reflection (≈ 20 s, 1 bullet on the conclusion slide)
> "The biggest lesson of this FYP wasn't the algorithm — it was the realization that the real engineering is the **7 800-configuration benchmark harness + multi-seed methodology** that made every tuning decision defensible. Without that, any claim of speedup is just a lucky run."

### 5.4 Conclusion (≈ 30 s)
**Delivered:**
- Header-only, STL-compatible DPQS for C++17 — one `#include`, works everywhere `std::sort` works.
- Adaptive optimizations: counting sort, run-merger, DNF for duplicates, introspective heapsort fallback.
- Parallel work-stealing execution with measured **4.72× peak speedup at 8 threads** on a 6 P-core + 4 E-core hybrid CPU.
- Benchmarks + VTune analysis that validate every design choice; the 8 → 16-thread regression is attributed to four stacked hardware ceilings (L3 latency, hybrid-core dilution, SMT cache sharing, mutex contention) — not our code.

---

## Slides to Prepare (~16 slides)

1. Title slide
2. Product pitch + call-site code example
3. **Research gap** — `std::sort` / pdqsort / ips4o / what we combine
4. **Methodology** — hardware, compiler, multi-seed protocol, warmup, outlier handling
5. Dispatch flowchart (Steps 0–3) + early-termination result
6. Counting-sort dispatch + `int8`/`int16` result
7. **The Step-4 recursion flowchart** (hero slide)
8. Insertion-sort leaves + random-int parity result
9. `try_merge_runs` + nearly/reverse/organ-pipe/sawtooth result
10. Pivot sampling — `sort5_network` diagram
11. Dual-pivot partition — three-region invariant
12. DNF fallback — duplicate handling + duplicates result
13. Sequential summary table — all patterns in one slide
14. Parallel work-stealing diagram + scaling chart (the plateau)
15. VTune L3-Bound breakdown
16. **Engineering (C++ craft) + Limitations + Reflection + Conclusion** (combined closing slide)

---

## Backup Slides (if asked)
- `sort5_network` — the exact 9 comparators
- `try_merge_runs` — run-detection heuristic
- Introsort depth bound derivation (`2·log₂ n · DELTA`)
- Counting-sort thresholds (1-byte vs 2-byte)
- Thread pool internals (Chase–Lev lineage, sticky victim, mutex vs lock-free trade-off)
- Compiler flags + exact hardware spec
- Full benchmark matrix (7 800+ configurations)
- Multi-seed protocol details — why median-of-medians (absorbs one unlucky seed)

---

## Presenter Notes — What to Emphasize

1. **"Cause, then effect."** Every guard in §2 is followed by the benchmark number it produces. The audience never has to wait to find out whether something worked.
2. **Prior art is mentioned inline, in one breath, as each technique appears** (Yaroslavskiy for dual-pivot, Timsort lineage for run-merging, Dijkstra for DNF, Musser for introsort depth bound, Blumofe–Leiserson for work-stealing). No dedicated literature section — we don't have the time budget and the walkthrough carries the attribution naturally.
3. **Sequential story ends at parity/wins on every pattern.** Then §3 shows we can scale it, §4 explains why we can't scale it *further*.
4. **VTune is a conclusion, not a detour.** It answers the question §3 leaves open.
5. **Every number is on the same hardware, same 10⁷ elements, same methodology** — and for random data the methodology is the industry-standard multi-seed protocol. State this once in §1.5 so the audience trusts every later table.
6. **The C++ craft slide at the end is a deliberate bookend** — we opened with "it looks like `std::sort`" and we close with "here's what it took to *really* look like `std::sort`."
