# Dual Pivot Quicksort Project TODO

## Phase 1 - Literature & Requirements (24 Oct - 8 Nov 2025)
- [ ] Consolidate dual-pivot quicksort theory and modern C++ sorting research.
  - Plan: Review Hoare, Yaroslavskiy, Aumuller & Dietzfelbinger, Wild, Musser, and Peters papers; capture key metrics (comparison bounds, cache models, adversarial performance); summarize implications for C++23 implementation constraints.
  - Output: Annotated bibliography + distilled requirements for algorithm design.
- [ ] Extract concrete C++23 language/library features needed for the implementation phase.
  - Plan: Prototype constexpr utilities, ranges-based interfaces, and allocator policies; document compiler flags for Clang, GCC, MSVC; note portability concerns across Apple/Windows builds.
  - Output: Feature checklist and compiler configuration matrix.
- [ ] Finalize project governance assets (timeline, success metrics, risk register).
  - Plan: Translate schedule milestones (proposal, interim, final report/presentation) into a shared tracker; define KPIs (speedup targets vs std::sort, cache-miss reduction, stability); log risks (hardware access, counter tooling) with mitigation steps.
  - Output: Updated plan baseline for supervisor check-in.

## Phase 2 - Core Implementation (9 Nov - 13 Dec 2025)
- [ ] Implement Yaroslavskiy's asymmetric dual-pivot quicksort variant in templated C++23.
  - Plan: Create dual_pivot.hpp/.cpp with ranges-friendly entry points; support iterators/comparators; enforce introspective fallback thresholds; add configurable insertion-sort cutoff; validate with unit tests on primitive and custom types.
  - Output: Baseline DPQS module checked into version control with tests.
- [ ] Implement adaptive dual-pivot model with dynamic pivot sampling heuristics.
  - Plan: Add strategy pattern for median-of-three, tertiles-of-five, and randomized sampling; expose knobs via config struct; log partition statistics for later analysis; ensure exception safety and noexcept guarantees.
  - Output: Strategy-aware DPQS variant toggleable via API.
- [ ] Build diagnostic instrumentation and parameter tuning hooks.
  - Plan: Attach counters for comparisons, swaps, scanned elements; guard with constexpr bool enable_metrics; provide CLI/JSON config for recursion depth, sampling size, insertion cutoff; integrate with logging subsystem.
  - Output: Instrumented build ready for benchmarking harness.
- [ ] Harden correctness with automated tests and CI scripts.
  - Plan: Use Catch2/GoogleTest (existing preference) to cover sorted, reverse, duplicates, and random datasets up to 10^6 elements; include fuzz tests comparing against std::sort; wire tests into CMake/Ninja targets for all compilers.
  - Output: Passing test suite demonstrating functional parity with STL sorts.

## Phase 3 - Benchmarking & Profiling (14 Dec 2025 - 5 Jan 2026)
- [ ] Implement data generator covering four distributions and size sweep (10^3-10^8 elements).
  - Plan: Write reusable generator module; persist seeds for reproducibility; stream datasets to binary blobs to avoid regeneration cost; document storage footprint.
  - Output: Reusable dataset assets + generator scripts under version control.
- [ ] Stand up cross-platform benchmarking harness on macOS (M2) and Windows/WSL (i5-12600KF).
  - Plan: Script builds with CMake presets per platform; ensure consistent compiler optimizations; automate deployment via SSH/remote PowerShell if needed; capture hardware/OS metadata per run.
  - Output: Repeatable benchmark pipeline across both target systems.
- [ ] Collect comparative metrics against baseline algorithms (std::sort, std::stable_sort, std::partial_sort, PDQSort).
  - Plan: Run full matrix (distribution x size x algorithm x platform); measure wall-clock time, comparisons, swaps, L1/L2 misses, branch mispredicts via Apple Instruments and PAPI/VTune; store raw CSV traces under results/.
  - Output: Benchmark dataset supporting empirical analysis.
- [ ] Analyze preliminary trends to feed interim reporting.
  - Plan: Use Python notebooks/R scripts to compute speedups, cache-efficiency ratios, and variance; flag anomalies for rerun; summarize top-line findings for slides.
  - Output: Initial plots + talking points for interim presentation.

## Phase 4 - Analysis & Interim Deliverables (6 Jan - 9 Jan 2026)
- [ ] Fit theoretical models to empirical data.
  - Plan: Regress runtime vs n log n to estimate alpha/beta constants; compare scanned-elements predictions with measured cache misses; document deviations and hypothesize causes.
  - Output: Analytical write-up bridging theory and measurements.
- [ ] Draft interim report and presentation video.
  - Plan: Structure report around objectives, methodology, results, risks; embed benchmark tables/graphs; record 10-12 minute walkthrough highlighting insights and next steps; collect supervisor feedback before submission.
  - Output: Interim report PDF + presentation video ready for 9 Jan deadline.

## Phase 5 - Extended Analysis & Refinement (10 Jan - 5 Apr 2026)
- [ ] Perform deeper cache and branch-behavior investigations.
  - Plan: Use perf/VTune/Apple Instruments to trace memory traffic; experiment with block partitioning tweaks; document trade-offs in scanned-elements budget.
  - Output: Advanced profiling appendix + recommendations for cache-aware tuning.
- [ ] Optimize adaptive heuristics and finalize parameter defaults.
  - Plan: Explore dynamic switching between sampling strategies based on observed imbalance; tune insertion-sort thresholds via auto-tuning script; validate gains across datasets.
  - Output: Finalized configuration set ready for potential STL proposal.
- [ ] Cleanup codebase and prepare reusable library artefact.
  - Plan: Refactor modules for readability, add Doxygen comments, ensure CMake install targets; tag release candidate builds.
  - Output: Polished code + documentation bundle.

## Phase 6 - Final Reporting & Presentation (6 Apr - 22 Apr 2026)
- [ ] Write final project report (due 10 Apr 2026).
  - Plan: Expand interim report with final experiments, analytical proofs, and integration guidance; include resource estimation appendix; run full proofreading and supervisor review cycle.
  - Output: Submission-ready final report PDF.
- [ ] Prepare and rehearse final presentation (11-22 Apr window).
  - Plan: Build concise slide deck covering motivation, methods, results, recommendations; rehearse Q&A scenarios; schedule rehearsal with supervisor before formal defense.
  - Output: Final slide deck + rehearsal notes.
- [ ] Package deliverables for submission and archival.
  - Plan: Ensure repository tags, benchmark data, scripts, and reports are organized; produce README describing reproduction steps; submit via official portal and back up to institutional storage.
  - Output: Complete submission archive delivered by 22 Apr 2026.

## Cross-Cutting Logistics & Risk Mitigation
- [ ] Secure hardware/software access and toolchain parity early.
  - Plan: Reserve lab time on M2 MacBook Air and Windows desktop; verify compiler/tool versions; document any licensing requirements for Instruments/VTune/PAPI.
  - Output: Access log + verification checklist.
- [ ] Establish automation for backups and raw-data integrity.
  - Plan: Configure Git + remote mirror; script nightly backup of results/ to external drive/cloud; checksum large datasets to detect corruption.
  - Output: Backup policy documentation + automation scripts.
- [ ] Maintain communication cadence with supervisor CAO Yixin.
  - Plan: Schedule bi-weekly syncs; share progress memos referencing this todo list; capture decisions and action items after each meeting.
  - Output: Meeting notes library supporting audit trail.
