# Video Presentation Script: Dual-Pivot Quicksort in C++23

**Target Duration:** 12-14 Minutes
**Format:** Screen Recording with Marp Slides
**Goal:** Showcase performance superiority and explain the architectural "Secret Sauce".

---

## Part 1: Introduction & Motivation (0:00 - 2:00)

### Slide 1: Title Card
*   **Visual:** Project Title, Student ID/Name, Supervisor Name. "Interim Assessment 2025/26".
*   **Audio Hook:**
    "Why did the Java standard library switch to Dual-Pivot Quicksort over 15 years ago, while C++ stuck with the single-pivot approach? Is it just inertia, or is there a fundamental difference in how these languages handle memory?

    Hello, I am LIN Zhanzhi, and for my Capstone Project, I set out to answer this question by implementing and benchmarking a modern, generic Dual-Pivot Quicksort for C++23."

### Slide 2: The Status Quo vs. The Innovation
*   **Visual:** Comparison split screen. `std::sort` (1 Pivot) vs Project (2 Pivots).
*   **Script:**
    "To understand the project, we first need to look at the status quo. Standard C++ `std::sort` typically uses Introsort—a hybrid single-pivot algorithm. It is excellent, but it was designed for an era when CPU cycles were the bottleneck.

    My implementation follows Vladimir Yaroslavskiy's Dual-Pivot approach. It uses *two* pivots to split the array into *three* parts. Now, intuitively, you might think this is slower because we do more comparisons per element. But on modern hardware, *computations are cheap, and memory writes are expensive*. By splitting into three parts, we move elements around less often. This 'Scanned Elements Model' suggests that saving memory bandwidth matters more than saving CPU instructions."

### Slide 3: Project Objectives
*   **Visual:** 3 Icons: Modernize (C++ Logo), Accelerate (Speedometer), Analyze (Microscope/Graph).
*   **Script:**
    "This brings us to the three core objectives of my Semester 1 work:
    First, **Modernize**: Create a truly generic implementation using C++20 Concepts, not just a C-style script for integers.
    Second, **Accelerate**: detailed benchmarking to prove—not just guess—that we can beat `std::sort`.
    And third, **Analyze**: I didn't just want to make it fast; I wanted to find the hardware limits, specifically investigating the 'Memory Wall' in parallel execution."

---

## Part 2: Sequential Performance Highlights (2:00 - 6:00)

### Slide 4: Baseline Results (Random Data)
*   **Visual:** Log-Log Graph of execution time ($10^3$ to $10^7$). Blue line (DPQS) below Red line (`std::sort`).
*   **Script:**
    "Let's jump straight into the results.
    Here you see the performance on purely random 64-bit integers, scaling from 1,000 to 10 million elements.
    The blue line is my Dual-Pivot implementation. As you can see, we maintain a consistent **10 to 15% speedup** over `std::sort` (the red line) across the entire range.
    This confirms the hypothesis: even with the overhead of generic templates, the improved cache behavior of 3-way partitioning translates to real-world gains in C++."

### Slide 5: The "Run Merger" Optimization (Structured Data)
*   **Visual:** Summary Table comparing DPQS vs `std::sort` for Organ Pipe and Sawtooth distributions, alongside runtime graphs.
*   **Script:**
    "But real-world data isn't always random. This is where 'Algorithmic Intelligence' kicks in.
    I implemented an Adaptive Run Merger, inspired by TimSort.
    As you can see in the table, for the 'Organ Pipe' distribution, `std::sort` treats it like random noise, but our implementation detects the structure and merges it, achieving a massive **28.7x speedup**.
    Similarly, for the 'Sawtooth' pattern, we achieve an **8.5x speedup**.
    By simply not re-sorting what is already sorted, we gain huge performance advantages on structured data."

### Slide 6: Robustness & The "Fat Partition"
*   **Visual:** Bullet points detailing the 3-Way Partitioning solution, followed by a runtime graph showing overlapping curves for 10%, 50%, and 90% duplicates.
*   **Script:**
    "Finally, for sequential sorting, we addressed robustness against the 'Fat Partition' problem.
    Standard Quicksort can degrade to $O(N^2)$ when faced with many duplicate pivots.
    My solution utilizes the 3-Way Partitioning structure. The middle region—between pivot 1 and pivot 2—naturally 'absorbs' all equal keys without extra work.
    The graph below proves this effectiveness: the runtime curves for 10%, 50%, and 90% duplicates completely overlap. The performance is **invariant**, meaning the algorithm remains fast and stable regardless of data repetition."

---

## Part 3: Parallel Scaling & Architecture (6:00 - 10:00)

### Slide 7: Parallel Architecture (V3 Work-Stealing)
*   **Visual:** Mermaid Schematic: "Owner Context" (LIFO) vs "Thief Context" (FIFO Stealing).
*   **Script:**
    "Moving to the second half of the semester: Parallelism.
    I implemented a V3 Work-Stealing architecture, visualized here.
    On the left, you see a 'Busy' thread (Core 1) processing its own deque in **LIFO** order, effectively using it as a stack. This keeps the hottest tasks in the CPU cache.
    On the right, an 'Idle' thread (Core 2) targets the **tail** of the busy thread's deque. It steals the oldest task—Task C—in **FIFO** order.
    This separation—Owner at the Head, Thief at the Tail—minimizes necessary locking, allowing the system to scale efficiently until the memory bus is saturated."

### Slide 8: Strong Scaling Results
*   **Visual:** Speedup Curve ($1 \to 16$ threads). Green zone (Linear) vs Red zone (Plateau).
*   **Script:**
    "So, how does it scale?
    On a standard Intel Core i7, we see perfect linear—and even super-linear—scaling up to 4 threads. Achieving 101% efficiency here suggests that using two cores effectively provided double the L2 cache, masking memory latency.
    By the time we hit 16 threads, we achieve a **5.24x speedup** over the sequential version. This cuts the sort time for 10 million integers down to just 100 milliseconds."

### Slide 9: Hitting the "Memory Wall"
*   **Visual:** The flat-lining curve emphasized. "Memory Bandwidth Saturation" label.
*   **Script:**
    "However, you will notice the curve flattens after 8 threads.
    This was a critical finding. It wasn't a software bug or lock contention.
    We hit the **Memory Wall**.
    Sorting is memory-intensive ($O(N)$ reads/writes). With 16 threads active, we saturated the physical memory controller's bandwidth. Adding more CPU cores stopped helping because we simply couldn't feed them data fast enough. This defines our roadmap for Semester 2."

---

## Part 4: Implementation & Future Path (10:00 - 13:00)

### Slide 10: Engineering Quality (C++23)
*   **Visual:** Brief code scroll. `template <std::sortable T>`.
*   **Script:**
    "I also want to touch on the engineering standard.
    This library is designed to be a drop-in replacement. It validates inputs relationships using C++20 Concepts, handles namespace safety to prevent collisions, and is fully 64-bit safe for arrays larger than 2 billion elements. It is not just an algorithm script; it is a library-grade codebase."

### Slide 11: Roadmap (Semester 2)
*   **Visual:** Roadmap Graphic. 1. SIMD/Vectorization, 2. Smart Scheduling, 3. PDQSort Comparison.
*   **Script:**
    "For Semester 2, the goal is to break that Memory Wall.
    The plan is three-fold:
    1.  **Vectorization**: Using AVX-512 and Non-Temporal Stores to bypass the cache and write directly to RAM, doubling our effective bandwidth.
    2.  **Memory-Aware Scheduling**: Making the thread pool smarter about *which* task it steals to preserve cache locality.
    3.  **Extended Benchmarking**: Comparing against the state-of-the-art PDQSort."

### Slide 12: Conclusion
*   **Visual:** Summary Bullets.
*   **Script:**
    "In conclusion, Semester 1 has been a success.
    We established that Dual-Pivot Quicksort isn't just for Java—it offers significant optimizations for C++ as well. We've achieved a **15% baseline speedup**, **27x structured speedup**, and a scalable parallel framework.
    We have the speed, we have the stability, and now we know exactly how to push the hardware further in the next phase.
    Thank you."

---

## 5. Technical Recording Notes
*   **Software**: Use OBS Studio for recording (Picture-in-Picture).
*   **Tone**: Confident, analytical, focusing on *why* things happen, not just *what* happened.
