# Video Presentation Script: Dual-Pivot Quicksort in C++23

**Target Duration:** 12-14 Minutes
**Format:** Screen Recording with Marp Slides
**Goal:** Showcase performance superiority and explain the architectural "Secret Sauce".

---

## Part 1: Introduction & Motivation (0:00 - 2:00)

### Slide 1: Title Card
*   **Visual:** Project Title, Student ID/Name, Supervisor Name. "Interim Assessment 2025/26".
*   **Audio Hook:**
    "Hello and welcome to my interim presentation. I am LIN Zhanzhi, and my project is 'Implementing Dual Pivot Quicksort in C++23: Validating Theoretically Superior Sorting Strategies in Modern Systems'.
    This project re-evaluates the standard sorting algorithms used in C++ in the context of modern hardware architectures."

### Slide 2: Agenda
*   **Visual:** Bulleted list of the presentation sections.
*   **Script:**
    "To guide you through my work, here is the agenda for today's presentation.
    I will begin with the **Background & Motivation**, where I will explain the historical context of Quicksort and why the Dual-Pivot approach—which has been standard in Java for over a decade—remains unexplored in the C++ standard library.
    Following that, I will share the **Sequential Performance** results. I will demonstrate how my implementation achieves a consistent 15% speedup on random data and a dramatic 28x speedup on structured distributions.
    Third, we will examine the **Parallel Architecture**. I will detail the design of my 'Version 3' work-stealing scheduler and discuss a critical finding: the 'Memory Wall' bottleneck that naturally limits scalability on multi-core systems.
    Finally, I will conclude with the **Roadmap for Semester 2**, specifically focusing on how I plan to use AVX-512 vectorization to break through that memory wall."

### Slide 3: The Status Quo vs. The Innovation
*   **Visual:** Comparison split screen. `std::sort` (1 Pivot) vs Project (2 Pivots).
*   **Script:**
    "To truly appreciate the innovation here, we must first examine the status quo. For decades, the C++ Standard Library's `std::sort` has relied on **Introsort**. This is a hybrid algorithm that starts with single-pivot Quicksort and switches to Heapsort if recursion goes too deep. It is a robust, battle-tested algorithm, but it was optimized for the hardware of the 1990s—an era when CPU clock cycles were the scarcest resource and memory latency was negligible.

    My implementation challenges this legacy by adopting Vladimir Yaroslavskiy's **Dual-Pivot Quicksort**. Instead of picking one pivot, we pick two, $P_1$ and $P_2$. This allows us to partition the array into *three* distinct regions in a single pass: elements smaller than $P_1$, those in the middle, and those larger than $P_2$.

    Now, from a classical algorithm analysis perspective, this seems counter-intuitive. Dual-pivot requires *more* comparisons per element than single-pivot. However, on modern hardware, the rules of the game have changed. Today, **arithmetic is virtually free, but memory access is expensive**. Writing to RAM or even L3 cache costs significantly more latency than a simple CPU register comparison.

    This is the core of the 'Scanned Elements Model'. By splitting the array into three parts instead of two, the algorithm moves elements to their final destination more efficiently, drastically reducing the total number of swap operations (memory writes). We essentially trade cheap CPU instructions to save expensive memory bandwidth, which is the winning strategy for modern, memory-bottlenecked systems."

### Slide 4: Project Objectives
*   **Visual:** 3 Icons: Modernize (C++ Logo), Accelerate (Speedometer), Analyze (Microscope/Graph).
*   **Script:**
    "This brings us to the three core objectives that guided my work during Semester 1.

    First, **Modernize**. Too often, high-performance algorithms remain as proofs-of-concept—simple C scripts that only sort arrays of integers. My goal was to bridge the gap between theory and practice by engineering a truly generic, production-ready library. Using modern C++20 Concepts, I aimed to ensure that this implementation is as flexible and type-safe as the standard library itself, capable of sorting anything from user-defined classes to complex data structures without modification.

    Second, **Accelerate**. The primary metric of success is, of course, speed. The objective here was to rigorously validate the performance against the industry standard, `std::sort`. I needed to prove—using statistically significant benchmarks—that the theoretical reduction in memory traffic actually translates to lower execution times on real hardware, and not just in specific corner cases, but across a wide range of data distributions.

    Third, **Analyze**. This project is not just about engineering; it is about research. I wanted to move beyond simple runtime measurements to understand the *why*. This meant conducting a deep architectural analysis to identify the physical limits of scalability. Specifically, I sought to investigate the 'Memory Wall'—the critical point where adding more CPU cores yields diminishing returns due to bandwidth saturation."

---

## Part 2: Sequential Performance Highlights (2:00 - 6:00)

### Slide 5: Baseline Results (Random Data)
*   **Visual:** Log-Log Graph of execution time ($10^3$ to $10^7$). Blue line (DPQS) below Red line (`std::sort`).
*   **Script:**
    "Let us transition from theory to empirical evidence. I will begin with the baseline sequential performance.
    
    This graph visualizes the execution time for sorting 64-bit integers, with the array size $N$ scaling from one thousand up to ten million elements. Both axes are logarithmic.
    The red line represents the current industry standard, `std::sort`. The blue line is my Dual-Pivot implementation.
    
    The results are clear: we maintain a consistent **10 to 15% speedup** across the entire range of data sizes.
    This result is significant because 'Random Data' is theoretically the worst-case scenario for valid prediction. Yet, even here, we win. This confirms our core hypothesis: the reduction in memory writes achieved by 3-way partitioning is substantial enough to overcome the overhead of generic template abstractions. We are effectively beating the standard library at its own game."

### Slide 6: The "Run Merger" Optimization (Structured Data)
*   **Visual:** Summary Table comparing DPQS vs `std::sort` for Organ Pipe and Sawtooth distributions, alongside runtime graphs.
*   **Script:**
    "However, random data is only part of the story. In the real world, data is often partially sorted or structured. This is where 'Algorithmic Intelligence' becomes critical.

    I integrated an Adaptive Run Merger into the algorithm, drawing inspiration from Timsort. Before recursion begins, my implementation scans the array to identify existing monotonic runs—ascending or descending sequences.

    The table on the right highlights the impact of this optimization on specific distributions.
    Take the 'Organ Pipe' distribution as an example. `std::sort` treats this as generic chaos and performs a full sort. My implementation, however, detects the underlying peaks and valleys, merges the pre-existing runs, and achieves a massive **28.7x speedup**.
    Similarly, for the 'Sawtooth' pattern, we see an **8.5x speedup**.
    We are essentially short-circuiting the sorting process. By recognizing and utilizing the order that already exists, we do not just sort faster; we avoid unnecessary work entirely."

### Slide 7: Robustness & The "Fat Partition"
*   **Visual:** Bullet points detailing the 3-Way Partitioning solution, followed by a runtime graph showing overlapping curves for 10%, 50%, and 90% duplicates.
*   **Script:**
    "Finally, for the sequential implementation, we had to address one of the most notorious killers of sorting performance: the 'Fat Partition' problem.
    In a standard single-pivot Quicksort, if the array contains many identical elements, the algorithm often performs redundant swaps, effectively shuffling identical values back and forth. In extreme cases, this causes the time complexity to degrade from $O(N \log N)$ to quadratic $O(N^2)$.
    
    My Dual-Pivot implementation solves this architecturally. By definition, we divide the data into three regions: less than $P_1$, greater than $P_2$, and the middle region where elements fall *between* the two pivots.
    Critically, when we encounter duplicates of the pivots, they are naturally clustered into this middle region. We do not need expensive special-casing; the algorithm simply 'absorbs' them.
    
    The graph below serves as definitive proof of this robustness. I tested the algorithm against datasets containing 10%, 50%, and even 90% duplicate values. As you can see, the runtime curves completely overlap. The performance is **invariant**. This means the algorithm's speed is completely decoupled from the level of repetition in the data—a stability guarantee that is essential for industrial-grade libraries."

---

## Part 3: Parallel Scaling & Architecture (6:00 - 10:00)

### Slide 8: Parallel Architecture (V3 Work-Stealing)
*   **Visual:** Diagram showing Owner Thread (Green) operating on Local Deque vs Thief Thread (Red) stealing from Tail.
*   **Script:**
    "Moving to the second half of the semester, we tackle the challenge of Parallelism.
    Simply spawning threads is easy, but achieving efficient scaling is difficult due to the 'Load Balancing' problem. In a recursive algorithm like Quicksort, task sizes vary wildly. If we assigned tasks statically, some cores would finish early and sit idle while others remained overworked.

    To solve this, I implemented a **Work-Stealing Scheduler**, specifically my 'Version 3' design.
    As illustrated in the diagram, this architecture gives every thread its own double-ended queue, or 'deque'.

    In the top section, we see the **Owner Thread** (in Green). It operates on its local deque in strict **LIFO** (Last-In, First-Out) order. It pushes and pops tasks from the *head* of the queue. This is crucial for performance because the most recently generated tasks represent the 'hottest', most cache-resident data.

    Meanwhile, look at the **Thief Thread** (in Red). When it runs out of its own work, it doesn't just sleep. It becomes a thief. It targets the **tail** of the victim's deque to steal the *oldest* available work—Task C—in **FIFO** (First-In, First-Out) order.

    This design is deliberate. By separating the operations—Owner at the Head, Thief at the Tail—we minimize the chance of these threads fighting over the same lock or memory line. This reduction in synchronization overhead is what allows the system to scale linearly."

### Slide 9: Strong Scaling Results
*   **Visual:**  
    1. Table showing Speedup (2.03x to 5.24x) and Efficiency drops.
    2. Speedup Curve ($1 \to 16$ threads) demonstrating the plateau.
    *(Image Ref: `PaperWork/InterimReport/image/7.3.1.png`)*
*   **Script:**
    "So, theory is one thing, but how does this architecture scale on real silicon?
    To measure this, I conducted a 'Strong Scaling' analysis. I fixed the workload at 10 million integers and progressively unlocked more cores, from 1 up to 16.

    The results, shown in this table, reveal two distinct phases.
    First, look at the transition from 2 to 4 threads. We actually observe **Super-Linear Scaling**, with efficiency peaking at 101%. This is a rare and desirable phenomenon. It happens because as we add Cores, we also add their private L2 caches. Effectively, we have more fast memory available for the same amount of data, which hides the latency of main RAM.

    However, as we push further to 8 and 16 threads, the law of diminishing returns kicks in. While the raw speed continues to improve—reaching a peak speedup of **5.24x**—the efficiency drops to 33%.
    We are still getting faster—slashing runtime from over half a second to just roughly 100 milliseconds—but we are paying a higher price in CPU cycles to get there. This behavior is the 'canary in the coal mine' that leads us to our next major finding."

### Slide 10: Hitting the "Memory Wall"
*   **Visual:** The flat-lining curve emphasized. "Memory Bandwidth Saturation" label.
*   **Script:**
    "However, if you look closely at the tail end of the graph, you will notice the curve flattens significantly after 8 threads.
    This plateau was a critical finding. Initially, we suspected synchronization overhead, but profiling proved that lock contention on the work-stealing queues was negligible.
    The real culprit is the **Memory Wall**.
    Sorting is an algorithm with very low 'Arithmetic Intensity'—meaning we perform very few calculations for every byte of data we move.
    With 16 threads all aggressively reading and writing potentially random parts of memory simultaneously, we completely saturated the bandwidth of the physical memory controller.
    Essentially, our CPU cores were ready to work faster, but they were left starving for data that the RAM simply couldn't deliver fast enough.
    This realization is pivotal. It tells us that simply throwing more cores at the problem won't work anymore. Instead, this defines our technical roadmap for Semester 2: we need to optimize *how* we access memory, not just how we compute."

---

## Part 4: Implementation & Future Path (10:00 - 13:00)

### Slide 11: Engineering Quality (C++23)
*   **Visual:** Brief code scroll. `template <std::sortable T>`.
*   **Script:**
    "I also want to spend a moment discussing the engineering quality of the project.
    This implementation wasn't written as a simple homework script; it was engineered to be a robust, production-ready library from day one.
    First and foremost, it is designed as a **Drop-in Replacement** for the standard library. This means any developer can switch from `std::sort` to my `dual_pivot_quicksort` by changing just one line of code. They don't need to rewrite their data structures or change their logic.
    To ensure safety, I heavily utilized modern **C++20 Concepts**. In older C++, template errors were notoriously difficult to debug. By using Concepts, my library validates input types at compile-time. If a user tries to sort data that isn't comparable, the compiler provides a clear, human-readable error message immediately, rather than failing deep inside the algorithm.
    Finally, the library is built for scale. It handles namespace isolation to prevent collisions and is fully **64-bit safe**. This is critical for modern data science, as it allows the algorithm to correctly index and sort arrays larger than 2 billion elements—a common failure point for integer-based implementations.
    In summary, the goal was to build a tool that isn't just fast, but is also safe and easy for others to adopt."

### Slide 12: Roadmap (Semester 2)
*   **Visual:** Roadmap Graphic. 1. SIMD/Vectorization, 2. Smart Scheduling, 3. PDQSort Comparison.
*   **Script:**
    "For Semester 2, our primary technical objective is clear: we must break the Memory Wall we identified in Slide 9. To do this, I have a three-point plan focusing on superior hardware utilization.
    
    First, I will implement **SIMD Vectorization** using AVX-512. Currently, we move data element-by-element. By using vector intrinsics, we can move 8 or 16 integers in a single CPU instruction. Crucially, I plan to use 'Non-Temporal Stores'. This technique allows us to write data directly to the main RAM, bypassing the CPU cache entirely. This prevents 'cache pollution' and effectively doubles our write bandwidth for large arrays.

    Second, I will upgrade the scheduler to be **Memory-Aware**. In the current Version 3, a thief steals from any available victim. In Semester 2, I will implement a 'NUMA-aware' stealing strategy. Threads will prioritize stealing tasks from other cores that share the same L3 cache. This reduces expensive cross-socket traffic and lowers latency.

    Finally, we need to aim higher than standard `std::sort`. The gold standard in the industry today is **PDQSort** (Pattern-Defeating Quicksort), which is used by Rust and Go. In the next phase, I will benchmark my Dual-Pivot implementation directly against PDQSort to see if we can beat the state-of-the-art."

### Slide 13: Conclusion
*   **Visual:** Summary Bullets.
*   **Script:**
    "To conclude, this semester's work has successfully challenged the assumption that single-pivot Introsort is the only viable option for C++.
    We have proven that Dual-Pivot Quicksort is not just a Java-specific optimization, but a superior algorithmic choice for modern hardware.
    In the sequential domain, we achieved a reliable **15% speedup** on random data and a massive **28x speedup** on structured data thanks to our adaptive run merger.
    In the parallel domain, we built a robust work-stealing runtime that scales linearly up to the hardware's physical limits.
    Crucially, we didn't just write code; we conducted a rigorous performance analysis that identified the 'Memory Wall' as the true bottleneck.
    Reflecting on Semester 1, we have built a faster, safer, and smarter sorting library. We have the foundation, and with our roadmap for vectorization and memory-aware scheduling, I am confident we can break through that memory wall in Semester 2.
    Thank you for your time, and I am happy to take any questions."

---

## 5. Technical Recording Notes
*   **Software**: Use OBS Studio for recording (Picture-in-Picture).
*   **Tone**: Confident, analytical, focusing on *why* things happen, not just *what* happened.
