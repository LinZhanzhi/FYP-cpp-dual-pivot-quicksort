/**
 * @file tune_inner_boundary.cpp
 * @brief Experiment to justify the inner boundary of 32 in mixed_insertion_sort
 * 
 * Questions to answer:
 * 1. Why is 32 the boundary where pin+pair kicks in?
 * 2. Why pin FIRST, then pair (not the reverse)?
 * 3. Why use BOTH strategies instead of just one?
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <fstream>

using Clock = std::chrono::high_resolution_clock;
using Nanoseconds = std::chrono::nanoseconds;

//=============================================================================
// INSERTION SORT VARIANTS
//=============================================================================

// Variant 1: Simple insertion (no pin, no pair)
template<typename T>
void insertion_simple(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    std::ptrdiff_t start = low;
    for (std::ptrdiff_t i; ++low < high; ) {
        T ai = a[i = low];
        while (--i >= start && ai < a[i]) {
            a[i + 1] = a[i];
        }
        a[i + 1] = ai;
    }
}

// Variant 2: Mixed with configurable boundary
template<typename T>
void insertion_mixed_boundary(T* a, std::ptrdiff_t low, std::ptrdiff_t high, int pair_count) {
    std::ptrdiff_t start = low;
    std::ptrdiff_t size = high - low;
    
    // pair_count = how many elements are handled by pair insertion at the end
    // If pair_count >= size, use simple insertion for entire array
    if (pair_count <= 0 || pair_count >= size) {
        // Simple insertion for entire array
        for (std::ptrdiff_t i; ++low < high; ) {
            T ai = a[i = low];
            while (--i >= start && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
        return;
    }
    
    std::ptrdiff_t end = high - pair_count;
    
    // Phase 1: Pin insertion for [low..end)
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
    
    // Phase 2: Pair insertion for [end..high)
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

// Variant 3: REVERSE order - Pair first, then Pin
template<typename T>
void insertion_pair_then_pin(T* a, std::ptrdiff_t low, std::ptrdiff_t high, int pin_count) {
    std::ptrdiff_t start = low;
    std::ptrdiff_t size = high - low;
    
    if (pin_count <= 0 || pin_count >= size) {
        // Simple insertion
        for (std::ptrdiff_t i; ++low < high; ) {
            T ai = a[i = low];
            while (--i >= start && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
        return;
    }
    
    std::ptrdiff_t end = low + pin_count;  // Pin handles LAST portion now
    
    // Phase 1: Pair insertion for first portion [low..end)
    // Need at least 2 sorted to start pair insertion
    if (high - low >= 2) {
        if (a[low + 1] < a[low]) std::swap(a[low], a[low + 1]);
        low += 2;
    }
    
    while (low + 1 < end) {
        std::ptrdiff_t i = low;
        T a1 = a[low], a2 = a[++low];
        ++low;
        
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
    
    // Handle odd element if any
    if (low < end) {
        T ai = a[low];
        std::ptrdiff_t i = low - 1;
        while (i >= start && ai < a[i]) {
            a[i + 1] = a[i];
            --i;
        }
        a[i + 1] = ai;
        ++low;
    }
    
    // Phase 2: Pin insertion for last portion [end..high)
    T pin = a[high - 1];
    
    for (std::ptrdiff_t i, p = high; low < high - 1; ++low) {
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
    
    // Insert pin element
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

// Variant 4: Pin only (no pair phase)
template<typename T>
void insertion_pin_only(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    std::ptrdiff_t start = low;
    T pin = a[high - 1];
    
    for (std::ptrdiff_t i, p = high; ++low < high - 1; ) {
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
    
    // Insert pin element
    T ai = a[high - 1];
    std::ptrdiff_t i = high - 2;
    while (i >= start && ai < a[i]) {
        a[i + 1] = a[i];
        --i;
    }
    a[i + 1] = ai;
}

// Variant 5: Pair only (no pin phase)
template<typename T>
void insertion_pair_only(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    std::ptrdiff_t start = low;
    
    if (high - low >= 2) {
        if (a[low + 1] < a[low]) std::swap(a[low], a[low + 1]);
        low += 2;
    }
    
    while (low + 1 < high) {
        std::ptrdiff_t i = low;
        T a1 = a[low], a2 = a[++low];
        ++low;
        
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

//=============================================================================
// BENCHMARK INFRASTRUCTURE
//=============================================================================

template<typename SortFn>
double benchmark_sort(SortFn sort_fn, int size, int iterations) {
    std::vector<int> original(size);
    std::vector<int> work(size);
    std::mt19937 rng(42);
    
    for (int i = 0; i < size; ++i) {
        original[i] = rng() % 10000;
    }
    
    // Warmup
    for (int i = 0; i < 100; ++i) {
        std::copy(original.begin(), original.end(), work.begin());
        sort_fn(work.data(), 0, size);
    }
    
    auto start = Clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::copy(original.begin(), original.end(), work.begin());
        sort_fn(work.data(), 0, size);
    }
    auto end = Clock::now();
    
    return std::chrono::duration_cast<Nanoseconds>(end - start).count() / (double)iterations;
}

int main() {
    int iterations = 100000;
    
    std::cout << "=" << std::string(78, '=') << "\n";
    std::cout << "EXPERIMENT 1: Why is 32 the inner boundary?\n";
    std::cout << "Find crossover where mixed (pin+pair) beats simple insertion\n";
    std::cout << "=" << std::string(78, '=') << "\n\n";
    
    std::vector<int> sizes = {16, 20, 24, 28, 32, 36, 40, 44, 48, 56, 64};
    
    std::cout << std::setw(6) << "Size" 
              << std::setw(14) << "Simple"
              << std::setw(14) << "Mixed(24)"
              << std::setw(14) << "Speedup"
              << std::setw(14) << "Winner"
              << "\n";
    std::cout << std::string(62, '-') << "\n";
    
    for (int size : sizes) {
        // Java formula: pair_count = 3 * ((size >> 5) << 3) = 24 for size 32-63
        int pair_count = 3 * ((size >> 5) << 3);
        
        double t_simple = benchmark_sort([](int* a, int low, int high) {
            insertion_simple(a, low, high);
        }, size, iterations);
        
        double t_mixed = benchmark_sort([pair_count](int* a, int low, int high) {
            insertion_mixed_boundary(a, low, high, pair_count);
        }, size, iterations);
        
        double speedup = t_simple / t_mixed;
        std::string winner = (speedup > 1.0) ? "MIXED" : "SIMPLE";
        
        std::cout << std::setw(6) << size
                  << std::setw(14) << std::fixed << std::setprecision(1) << t_simple
                  << std::setw(14) << t_mixed
                  << std::setw(14) << std::setprecision(2) << speedup << "x"
                  << std::setw(14) << winner
                  << "\n";
    }
    
    std::cout << "\n\n";
    std::cout << "=" << std::string(78, '=') << "\n";
    std::cout << "EXPERIMENT 2: Why Pin FIRST, then Pair (not reverse)?\n";
    std::cout << "Compare: [Pin→Pair] vs [Pair→Pin]\n";
    std::cout << "=" << std::string(78, '=') << "\n\n";
    
    std::vector<int> sizes2 = {32, 40, 48, 56, 64};
    
    std::cout << std::setw(6) << "Size"
              << std::setw(16) << "Pin→Pair"
              << std::setw(16) << "Pair→Pin"
              << std::setw(14) << "Ratio"
              << std::setw(14) << "Winner"
              << "\n";
    std::cout << std::string(66, '-') << "\n";
    
    for (int size : sizes2) {
        int pair_count = 3 * ((size >> 5) << 3);
        int pin_count = pair_count;  // Same split, different order
        
        double t_pin_pair = benchmark_sort([pair_count](int* a, int low, int high) {
            insertion_mixed_boundary(a, low, high, pair_count);
        }, size, iterations);
        
        double t_pair_pin = benchmark_sort([pin_count](int* a, int low, int high) {
            insertion_pair_then_pin(a, low, high, pin_count);
        }, size, iterations);
        
        double ratio = t_pair_pin / t_pin_pair;
        std::string winner = (ratio > 1.0) ? "Pin→Pair" : "Pair→Pin";
        
        std::cout << std::setw(6) << size
                  << std::setw(16) << std::fixed << std::setprecision(1) << t_pin_pair
                  << std::setw(16) << t_pair_pin
                  << std::setw(14) << std::setprecision(2) << ratio << "x"
                  << std::setw(14) << winner
                  << "\n";
    }
    
    std::cout << "\n\n";
    std::cout << "=" << std::string(78, '=') << "\n";
    std::cout << "EXPERIMENT 3: Why use BOTH strategies?\n";
    std::cout << "Compare: Pin only vs Pair only vs Mixed\n";
    std::cout << "=" << std::string(78, '=') << "\n\n";
    
    std::cout << std::setw(6) << "Size"
              << std::setw(14) << "PinOnly"
              << std::setw(14) << "PairOnly"
              << std::setw(14) << "Mixed"
              << std::setw(14) << "Winner"
              << "\n";
    std::cout << std::string(62, '-') << "\n";
    
    for (int size : sizes2) {
        int pair_count = 3 * ((size >> 5) << 3);
        
        double t_pin = benchmark_sort([](int* a, int low, int high) {
            insertion_pin_only(a, low, high);
        }, size, iterations);
        
        double t_pair = benchmark_sort([](int* a, int low, int high) {
            insertion_pair_only(a, low, high);
        }, size, iterations);
        
        double t_mixed = benchmark_sort([pair_count](int* a, int low, int high) {
            insertion_mixed_boundary(a, low, high, pair_count);
        }, size, iterations);
        
        std::string winner;
        if (t_mixed <= t_pin && t_mixed <= t_pair) winner = "MIXED";
        else if (t_pair <= t_pin) winner = "PAIR";
        else winner = "PIN";
        
        std::cout << std::setw(6) << size
                  << std::setw(14) << std::fixed << std::setprecision(1) << t_pin
                  << std::setw(14) << t_pair
                  << std::setw(14) << t_mixed
                  << std::setw(14) << winner
                  << "\n";
    }
    
    std::cout << "\n\n";
    std::cout << "=" << std::string(78, '=') << "\n";
    std::cout << "EXPERIMENT 4: Optimal pair_count sweep\n";
    std::cout << "For size=48, what's the best split between pin and pair?\n";
    std::cout << "=" << std::string(78, '=') << "\n\n";
    
    int test_size = 48;
    std::cout << "Size = " << test_size << "\n";
    std::cout << std::setw(12) << "pair_count"
              << std::setw(12) << "pin_count"
              << std::setw(14) << "Runtime(ns)"
              << "\n";
    std::cout << std::string(38, '-') << "\n";
    
    double best_time = 1e9;
    int best_pair_count = 0;
    
    for (int pair_count = 0; pair_count <= test_size; pair_count += 4) {
        int pin_count = test_size - pair_count;
        
        double t = benchmark_sort([pair_count](int* a, int low, int high) {
            insertion_mixed_boundary(a, low, high, pair_count);
        }, test_size, iterations);
        
        if (t < best_time) {
            best_time = t;
            best_pair_count = pair_count;
        }
        
        std::cout << std::setw(12) << pair_count
                  << std::setw(12) << pin_count
                  << std::setw(14) << std::fixed << std::setprecision(1) << t
                  << (pair_count == 24 ? " <-- Java default" : "")
                  << "\n";
    }
    
    std::cout << "\nBest: pair_count=" << best_pair_count 
              << " (" << std::fixed << std::setprecision(1) << best_time << " ns)\n";
    std::cout << "Java default: pair_count=24\n";
    
    std::cout << "\n\n";
    std::cout << "=" << std::string(78, '=') << "\n";
    std::cout << "SUMMARY\n";
    std::cout << "=" << std::string(78, '=') << "\n";
    
    return 0;
}
