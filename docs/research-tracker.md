# Phase 1 Research Tracker

## Bibliography & Reading Actions

| Ref ID | Source File | Citation / Focus | Key Insights for Project | Immediate Actions |
|--------|-------------|------------------|--------------------------|-------------------|
| B1 | PaperWork/Project Proposal/source/2007-9737-cys-28-03-1041.pdf | Nebel & Wild (2007) on cache-aware quicksort variants | Introduces empirical cache modeling techniques that inspired the scanned-elements metric. | Extract methodology for measuring cache misses and adapt counters list for benchmarking scripts. |
| B2 | PaperWork/Project Proposal/source/2106.05123v1.pdf | Peters (2021) Pattern-Defeating Quicksort preprint | Details adaptive sampling heuristics for adversarial inputs. | Summarize pivot reselection triggers to reuse inside adaptive dual-pivot strategy module. |
| B3 | PaperWork/Project Proposal/source/alopez2014-seminar-qsort.pdf | Seminar report comparing classic quicksort optimizations | Gives historical comparison of median-of-three, introspective fallback, and insertion cutoffs. | Capture recommended thresholds to seed DPQS tuning parameters. |
| B4 | PaperWork/Project Proposal/source/DualPivotQuicksort.pdf | Yaroslavskiy (2009) asymmetric dual-pivot quicksort | Core algorithm specification adopted by Java 7. | Produce annotated pseudocode -> implementation checklist for templated C++23 version. |
| B5 | PaperWork/Project Proposal/source/optimal partitioning for dual-pivot quicksort.pdf | Wild (2012) optimal partitioning analysis | Analyzes pivot spacing and scanned-elements trade-offs. | Translate optimal pivot distance recommendations into config defaults and instrumentation metrics. |
| B6 | PaperWork/Project Proposal/source/martinez-nebel-wild-2019.pdf | Martinez, Nebel, Wild (2019) multi-pivot asymptotics | Extends theory beyond two pivots to show diminishing returns. | Note comparison-count bounds for literature review discussion and justify scope focusing on dual pivots. |
| B7 | PaperWork/Project Proposal/source/multipivotQuicksort.pdf | Aumüller & Dietzfelbinger (2015) rigorous DPQS analysis | Provides 1.8·n·ln n comparison bound derivation. | Recreate recurrence derivation for analytical section and validate via regression fits. |
| B8 | PaperWork/Project Proposal/source/wild-dissertation.pdf | Wild dissertation on scanned-elements model | Comprehensive treatment of cache-behavior modeling for partitioning algorithms. | Derive metric formulas and define hardware counter mapping (PAPI/Instruments) for benchmarking phase. |
| B9 | PaperWork/Project Proposal/source/wild-nebel-2012.pdf | Wild & Nebel (2012) on empirical DPQS results | Presents benchmark methodology and hardware setup. | Reproduce dataset distributions and measurement protocol for comparability. |
| B10 | PaperWork/Project Proposal/source/Why Is Dual-Pivot Quicksort Fast.pdf | Oracle whitepaper summarizing Java adoption | Explains engineering decisions transitioning std::sort to DPQS-style pipeline. | Distill talking points for motivation/background chapter and supervisor updates. |
| B11 | PaperWork/Project Proposal/source/paper.pdf | Likely Hoare 1962 or related foundational text (verify) | Needed for original quicksort reference. | Confirm exact citation metadata and add to bibliography with proper BibTeX entry. |
| B12 | PaperWork/Project Proposal/source/alopez2014-seminar-qsort.pdf & DualPivotQuicksort.java | Java reference implementation | Serves as baseline for feature parity tests. | Mine implementation details (partition loops, fallback to insertion sort) to ensure C++ version parity. |

*Next action:* For each ref, capture BibTeX (title, venue, year, DOI) in Zotero/Markdown table and link to todo items Phase 1 lines 4-13 in docs/todo-list.md.

## Risk Log (Phase 1 Focus)

| Risk ID | Description | Impact | Likelihood | Mitigation / Owner | Status |
|---------|-------------|--------|------------|---------------------|--------|
| R1 | Missing metadata (title/author) for several PDFs (table extraction failed). | Delays bibliography submission and citation accuracy. | Medium | Manually inspect each PDF front matter and update tracker with proper citation; automate with pdfinfo where possible. Owner: LIN Zhanzhi. | Open |
| R2 | Limited access to profiling tools (Apple Instruments, VTune, PAPI) during early research. | Could block cache-analysis planning and metric definitions. | Medium | Schedule hardware/tool access during Phase 1; document fallbacks (perf/Timer runs) in tracker. Owner: LIN Zhanzhi. | Open |
| R3 | Hardware scheduling conflicts for MacBook Air and Windows desktop. | May slip benchmarking milestones if not reserved early. | Medium | Reserve lab time now and log in tracker; consider remote access plan. Owner: LIN Zhanzhi. | Open |
| R4 | Scope creep into multi-pivot algorithms beyond dual-pivot focus. | Dilutes effort and extends literature review timeline. | Low | Keep tracker limited to references needed for objectives; escalate to supervisor if scope change requested. Owner: LIN Zhanzhi. | Open |
| R5 | Data integrity issues for large benchmark datasets (>10^8 elements) when generated later. | Risk of reruns and inconsistent reporting. | Medium | Define checksum/backup process in Phase 1 documentation so tooling is ready before benchmarking. Owner: LIN Zhanzhi. | Open |

*Tracker maintenance cadence:* Update bibliography status and risk log after each bi-weekly supervisor sync; link updates back to docs/todo-list.md Phase 1 checkboxes.
