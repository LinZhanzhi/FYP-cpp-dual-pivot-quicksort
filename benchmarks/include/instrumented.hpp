#ifndef BENCHMARK_INSTRUMENTED_HPP
#define BENCHMARK_INSTRUMENTED_HPP

#include <utility>
#include <cstddef>
#include <iostream>
#include <atomic>

template <typename T>
struct Instrumented {
    T value;

    // Static counters
    static inline std::atomic<size_t> comparisons{0};
    static inline std::atomic<size_t> swaps{0};
    static inline std::atomic<size_t> assignments{0};

    static void reset() {
        comparisons = 0;
        swaps = 0;
        assignments = 0;
    }

    Instrumented() : value() {}
    Instrumented(T v) : value(v) {}

    // Count Copies and Moves
    Instrumented(const Instrumented& other) : value(other.value) {
        assignments++;
    }
    Instrumented(Instrumented&& other) noexcept : value(std::move(other.value)) {
        assignments++;
    }
    Instrumented& operator=(const Instrumented& other) {
        assignments++;
        value = other.value;
        return *this;
    }
    Instrumented& operator=(Instrumented&& other) noexcept {
        assignments++;
        value = std::move(other.value);
        return *this;
    }

    // Relational operators
    bool operator<(const Instrumented& other) const {
        comparisons++;
        return value < other.value;
    }
    bool operator>(const Instrumented& other) const {
        comparisons++;
        return value > other.value;
    }
    bool operator<=(const Instrumented& other) const {
        comparisons++;
        return value <= other.value;
    }
    bool operator>=(const Instrumented& other) const {
        comparisons++;
        return value >= other.value;
    }
    bool operator==(const Instrumented& other) const {
        comparisons++;
        return value == other.value;
    }
    bool operator!=(const Instrumented& other) const {
        comparisons++;
        return value != other.value;
    }

    // Mixed comparison with raw T isn't strictly necessary for sorting array of Instrumented<T>
    // but good for completeness if needed.

    // Friend swap - this is the key for counting swaps
    friend void swap(Instrumented& a, Instrumented& b) {
        swaps++;
        using std::swap;
        swap(a.value, b.value);
    }
};

#endif
