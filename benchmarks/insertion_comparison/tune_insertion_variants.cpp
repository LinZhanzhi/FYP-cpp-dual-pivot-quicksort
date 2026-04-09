/**
 * @file tune_insertion_variants.cpp
 * @brief Benchmark comparing different insertion sort strategies
 * 
 * This experiment compares:
 * 1. Simple insertion (no optimizations)
 * 2. Simple insertion with prefetch + branch hints
 * 3. Pin insertion only
 * 4. Pair insertion only
 * 5. Mixed insertion (pin + pair)
 * 
 * Goal: Find optimal inner boundaries for each strategy
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <cstring>

// Compiler intrinsics
#if defined(__GNUC__) || defined(__clang__)
    #define DPQS_PREFETCH_READ(addr) __builtin_prefetch((addr), 0, 3)
    #define DPQS_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define DPQS_LIKELY(x) __builtin_expect(!!(x), 1)
#else
    #define DPQS_PREFETCH_READ(addr)
    #define DPQS_UNLIKELY(x) (x)
    #define DPQS_LIKELY(x) (x)
#endif

using Clock = std::chrono::high_resolution_clock;
using Nanoseconds = std::chrono::nanoseconds;

//=============================================================================
// INSERTION SORT VARIANTS
//=============================================================================

/**
 * @brief Variant 1: Naive insertion sort (baseline)
 * No prefetch, no branch hints, basic implementation
 */
template<typename T>
void insertion_naive(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    for (std::ptrdiff_t i, k = low; ++k < high; ) {
        T ai = a[i = k];
        if (ai < a[i - 1]) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

/**
 * @brief Variant 2: Insertion sort with prefetch + branch hints
 * Our optimized simple insertion
 */
template<typename T>
void insertion_prefetch(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    for (std::ptrdiff_t i, k = low; ++k < high; ) {
        T ai = a[i = k];
        
        if (DPQS_LIKELY(k + 1 < high)) {
            DPQS_PREFETCH_READ(&a[k + 1]);
        }
        
        if (DPQS_UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

/**
 * @brief Variant 3: Pin insertion sort only
 * Separates elements around a pin without pair processing
 */
template<typename T>
void insertion_pin_only(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    std::ptrdiff_t start = low;
    T pin = a[high - 1];  // Pin element
    
    for (std::ptrdiff_t i, p = high; ++low < high - 1; ) {
        T ai = a[i = low];
        
        if (ai < a[i - 1]) {
            // Small element: standard insertion
            a[i] = a[i - 1];
            --i;
            while (--i >= start && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        } else if (p > i && pin < ai) {
            // Large element: swap to end
            while (pin < a[--p]);
            if (p > i) {
                ai = a[p];
                a[p] = a[i];
            }
            while (--i >= start && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
    
    // Insert the pin element itself
    T ai = a[high - 1];
    std::ptrdiff_t i = high - 2;
    while (i >= start && ai < a[i]) {
        a[i + 1] = a[i];
        --i;
    }
    a[i + 1] = ai;
}

/**
 * @brief Variant 4: Pair insertion sort only
 * Processes elements two at a time (no pin phase)
 */
template<typename T>
void insertion_pair_only(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    std::ptrdiff_t start = low;
    
    // First pass: ensure we have at least 2 sorted at start
    if (high - low >= 2) {
        if (a[low + 1] < a[low]) {
            std::swap(a[low], a[low + 1]);
        }
        low += 2;
    }
    
    // Process pairs
    for (std::ptrdiff_t i; low + 1 < high; low += 2) {
        T a1 = a[i = low], a2 = a[low + 1];
        
        if (a2 < a1) {
            // a1 > a2: Insert a1 first (larger), then a2
            while (--i >= start && a1 < a[i]) {
                a[i + 2] = a[i];
            }
            a[++i + 1] = a1;
            while (--i >= start && a2 < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = a2;
        } else if (a1 < a[i - 1]) {
            // a1 <= a2: Insert a2 first, then a1
            while (--i >= start && a2 < a[i]) {
                a[i + 2] = a[i];
            }
            a[++i + 1] = a2;
            while (--i >= start && a1 < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = a1;
        }
    }
    
    // Handle odd element
    if (low < high) {
        T ai = a[low];
        std::ptrdiff_t i = low - 1;
        while (i >= start && ai < a[i]) {
            a[i + 1] = a[i];
            --i;
        }
        a[i + 1] = ai;
    }
}

/**
 * @brief Variant 5: Mixed insertion (pin + pair) — Current implementation
 * Uses pin insertion for first portion, pair insertion for remainder
 */
template<typename T>
void insertion_mixed(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    std::ptrdiff_t start = low;
    std::ptrdiff_t size = high - low;
    std::ptrdiff_t end = high - 3 * ((size >> 5) << 3);  // Transition point
    
    if (end == high) {
        // Tiny array: simple insertion
        for (std::ptrdiff_t i; ++low < end; ) {
            T ai = a[i = low];
            while (--i >= start && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Phase 1: Pin insertion
        T pin = a[end];
        
        for (std::ptrdiff_t i, p = high; ++low < end; ) {
            T ai = a[i = low];
            
            if (ai < a[i - 1]) {
                a[i] = a[i - 1];
                --i;
                while (--i >= start && ai < a[i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            } else if (p > i && pin < ai) {
                while (pin < a[--p]);
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }
                while (--i >= start && ai < a[i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }
        
        // Phase 2: Pair insertion
        for (std::ptrdiff_t i; low < high; ++low) {
            T a1 = a[i = low], a2 = a[++low];
            
            if (a2 < a1) {
                while (--i >= start && a1 < a[i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;
                while (--i >= start && a2 < a[i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;
            } else if (a1 < a[i - 1]) {
                while (--i >= start && a2 < a[i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;
                while (--i >= start && a1 < a[i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

//=============================================================================
// BENCHMARK INFRASTRUCTURE
//=============================================================================

struct BenchmarkResult {
    std::string variant;
    int size;
    double time_ns;  // Nanoseconds per sort
};

template<typename SortFn>
double benchmark_sort(SortFn sort_fn, int size, int iterations) {
    std::vector<int> original(size);
    std::vector<int> work(size);
    std::mt19937 rng(42);  // Fixed seed for reproducibility
    
    // Generate random data
    for (int i = 0; i < size; ++i) {
        original[i] = rng() % 10000;
    }
    
    // Warmup
    for (int i = 0; i < 100; ++i) {
        std::copy(original.begin(), original.end(), work.begin());
        sort_fn(work.data(), 0, size);
    }
    
    // Timed runs
    auto start = Clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::copy(original.begin(), original.end(), work.begin());
        sort_fn(work.data(), 0, size);
    }
    auto end = Clock::now();
    
    double total_ns = std::chrono::duration_cast<Nanoseconds>(end - start).count();
    return total_ns / iterations;
}

int main() {
    std::vector<int> sizes = {8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 70, 80};
    int iterations = 100000;  // High iteration count for small arrays
    
    std::vector<BenchmarkResult> results;
    
    std::cout << "Benchmarking insertion sort variants...\n";
    std::cout << "Iterations per size: " << iterations << "\n\n";
    
    // Header
    std::cout << std::setw(6) << "Size"
              << std::setw(14) << "Naive"
              << std::setw(14) << "Prefetch"
              << std::setw(14) << "Pin"
              << std::setw(14) << "Pair"
              << std::setw(14) << "Mixed"
              << "\n";
    std::cout << std::string(76, '-') << "\n";
    
    for (int size : sizes) {
        double t_naive = benchmark_sort(insertion_naive<int>, size, iterations);
        double t_prefetch = benchmark_sort(insertion_prefetch<int>, size, iterations);
        double t_pin = benchmark_sort(insertion_pin_only<int>, size, iterations);
        double t_pair = benchmark_sort(insertion_pair_only<int>, size, iterations);
        double t_mixed = benchmark_sort(insertion_mixed<int>, size, iterations);
        
        results.push_back({"naive", size, t_naive});
        results.push_back({"prefetch", size, t_prefetch});
        results.push_back({"pin", size, t_pin});
        results.push_back({"pair", size, t_pair});
        results.push_back({"mixed", size, t_mixed});
        
        std::cout << std::setw(6) << size
                  << std::setw(14) << std::fixed << std::setprecision(1) << t_naive
                  << std::setw(14) << t_prefetch
                  << std::setw(14) << t_pin
                  << std::setw(14) << t_pair
                  << std::setw(14) << t_mixed
                  << "\n";
    }
    
    // Write CSV for visualization
    std::ofstream csv("insertion_comparison.csv");
    csv << "variant,size,time_ns\n";
    for (const auto& r : results) {
        csv << r.variant << "," << r.size << "," << r.time_ns << "\n";
    }
    csv.close();
    
    std::cout << "\n\nResults saved to insertion_comparison.csv\n";
    
    // Analysis: Find crossover points
    std::cout << "\n=== ANALYSIS ===\n";
    std::cout << "Looking for crossover points where strategy changes winner...\n\n";
    
    for (size_t i = 0; i < sizes.size(); ++i) {
        int size = sizes[i];
        double t_prefetch = 0, t_mixed = 0;
        for (const auto& r : results) {
            if (r.size == size) {
                if (r.variant == "prefetch") t_prefetch = r.time_ns;
                if (r.variant == "mixed") t_mixed = r.time_ns;
            }
        }
        
        double ratio = t_mixed / t_prefetch;
        std::string winner = (ratio < 1.0) ? "MIXED" : "PREFETCH";
        std::cout << "Size " << std::setw(3) << size 
                  << ": prefetch=" << std::setw(8) << std::fixed << std::setprecision(1) << t_prefetch
                  << "ns, mixed=" << std::setw(8) << t_mixed 
                  << "ns, ratio=" << std::setprecision(3) << ratio
                  << " -> " << winner << "\n";
    }
    
    return 0;
}
