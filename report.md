# Progress Report: Dual-Pivot Quicksort Implementation and Analysis

**Date:** December 10, 2025

## 1. Executive Summary
This report outlines the progress made on the Final Year Project regarding the implementation, benchmarking, and analysis of the Dual-Pivot Quicksort algorithm. The primary focus has been on porting the reference Java implementation to C++, establishing a benchmarking framework, and developing a web-based interface for experiment management.

## 2. Completed Work

### 2.1. Algorithm Implementation
-   **Java to C++ Transcription**: Successfully transcribed the Dual-Pivot Quicksort implementation from the OpenJDK Java library into C++.
-   **Verification**: The C++ implementation has been verified against standard sorting algorithms to ensure correctness.
-   **Integration**: The algorithm is integrated into a modular project structure, allowing for easy testing and comparison.

### 2.2. Benchmarking Infrastructure
-   **C++ Benchmarking Suite**: Developed a comprehensive C++ benchmarking tool capable of generating various data patterns (Random, Nearly Sorted, Reverse Sorted, Many Duplicates, etc.) and data types (`int`, `double`).
-   **Automation**: Created scripts and Makefiles to automate the build and execution process.

### 2.3. Web Interface for Experiment Management
-   **Frontend Development**: Built a web-based User Interface using Vue.js to manage benchmark runs.
    -   Features include: Selecting algorithms, data types, patterns, and sizes.
    -   Real-time status updates and ability to run, rerun, or delete test results.
    -   "Runtime" display for quick performance feedback.
-   **Backend Server**: Implemented a Python-based HTTP server to handle API requests, execute the C++ benchmark binaries, and parse CSV results on the fly.

## 3. Current Status of Data Collection
-   **Initial Data**: Preliminary performance data has been collected for various test cases.
-   **Protocol Limitations**: The current data collection protocol relies on a single execution per test case. This approach is susceptible to system noise and outliers, making the results less statistically robust.

## 4. Future Work & Improvement Plan

### 4.1. Benchmarking Protocol Refinement
-   **Scientific Rigor**: The immediate next step is to research and implement a more scientifically rigorous benchmarking protocol.
-   **Proposed Changes**:
    -   Implement "warm-up" runs to stabilize the cache and branch predictor state.
    -   Perform multiple iterations per test case.
    -   Calculate statistical metrics (Mean, Median, Standard Deviation) rather than relying on a single data point.

### 4.2. Data Analysis and Visualization
-   **Frontend Expansion**: The web interface will be expanded to include data analysis tools.
-   **Visualization**: Integration of graphing libraries to plot performance curves (e.g., Runtime vs. Input Size) directly in the browser, facilitating easier comparison between Dual-Pivot Quicksort and `std::sort`.

### 4.3. Theoretical Modeling
-   **Literature Review**: A deeper dive into the theoretical foundations of Dual-Pivot Quicksort is required.
-   **Modeling**: Work will focus on understanding the mathematical modeling of the algorithm's complexity and behavior to correlate theoretical expectations with empirical results.

### 4.4. Publication Goals
-   **Symposium Paper**: Aim to draft and submit a paper to a relevant symposium this year.
-   **Feasibility**: Based on the current progress of implementation and the planned improvements to the benchmarking protocol, producing a high-quality paper within the timeframe is deemed achievable.

### 4.5. Optimization and Tuning Strategy
-   **SIMD Acceleration**: Plan to investigate the C++ implementation to identify hotspots suitable for SIMD (Single Instruction, Multiple Data) instructions to enhance performance.
-   **Threshold Tuning**: Considering fine-tuning the array size thresholds (e.g., for switching to Insertion Sort) to maximize performance.
-   **Generalization vs. Overfitting**: Acknowledging the risk that aggressive fine-tuning might overfit the algorithm to the specific architecture of the development workstation. Future testing will aim to verify performance stability across different machine configurations to ensure general applicability.
