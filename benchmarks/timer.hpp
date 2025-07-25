#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace benchmark_timer {

struct TimingResult {
    std::vector<double> times_ms;
    double mean_ms;
    double median_ms;
    double std_dev_ms;
    double min_ms;
    double max_ms;
};

class BenchmarkTimer {
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    
public:
    // Time a single execution
    template<typename Func>
    double time_execution(Func&& func) {
        auto start = Clock::now();
        func();
        auto end = Clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        return duration.count() / 1e6;  // Convert to milliseconds
    }
    
    // Run benchmark multiple times and collect statistics
    template<typename Func>
    TimingResult benchmark(Func&& func, int iterations = 50) {
        std::vector<double> times;
        times.reserve(iterations);
        
        // Warmup runs to stabilize caches and CPU frequency
        for (int i = 0; i < 5; ++i) {
            func();
        }
        
        // Actual timing runs
        for (int i = 0; i < iterations; ++i) {
            times.push_back(time_execution(func));
        }
        
        return calculate_statistics(times);
    }
    
    // Time sorting algorithm with copy overhead measurement
    template<typename Container, typename SortFunc>
    TimingResult benchmark_sort(const Container& original_data, SortFunc sort_func, int iterations = 50) {
        std::vector<double> times;
        times.reserve(iterations);
        
        // Warmup
        for (int i = 0; i < 5; ++i) {
            Container data_copy = original_data;
            sort_func(data_copy);
        }
        
        // Actual benchmarking
        for (int i = 0; i < iterations; ++i) {
            Container data_copy = original_data;
            
            auto start = Clock::now();
            sort_func(data_copy);
            auto end = Clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            times.push_back(duration.count() / 1e6);
        }
        
        return calculate_statistics(times);
    }
    
private:
    TimingResult calculate_statistics(std::vector<double>& times) {
        TimingResult result;
        result.times_ms = times;
        
        if (times.empty()) {
            result.mean_ms = result.median_ms = result.std_dev_ms = 0.0;
            result.min_ms = result.max_ms = 0.0;
            return result;
        }
        
        // Sort for median calculation
        std::sort(times.begin(), times.end());
        
        // Calculate statistics
        result.min_ms = times.front();
        result.max_ms = times.back();
        
        // Mean
        result.mean_ms = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        
        // Median
        size_t mid = times.size() / 2;
        if (times.size() % 2 == 0) {
            result.median_ms = (times[mid - 1] + times[mid]) / 2.0;
        } else {
            result.median_ms = times[mid];
        }
        
        // Standard deviation
        double variance = 0.0;
        for (double time : times) {
            double diff = time - result.mean_ms;
            variance += diff * diff;
        }
        variance /= times.size();
        result.std_dev_ms = std::sqrt(variance);
        
        return result;
    }
};

// Instrumented timing for operation counting
struct InstrumentedResult {
    TimingResult timing;
    size_t comparisons;
    size_t swaps;
    size_t memory_accesses;
};

template<typename T>
class InstrumentedCompare {
private:
    mutable size_t* counter;
    std::less<T> comp;
    
public:
    InstrumentedCompare(size_t* comp_counter) : counter(comp_counter) {}
    
    bool operator()(const T& a, const T& b) const {
        if (counter) ++(*counter);
        return comp(a, b);
    }
};

// Memory access counter (simplified)
template<typename T>
class MemoryAccessCounter {
private:
    size_t* access_counter;
    
public:
    MemoryAccessCounter(size_t* counter) : access_counter(counter) {}
    
    // This would be more complex in a real implementation
    // For now, it's a placeholder for memory access tracking
};

} // namespace benchmark_timer