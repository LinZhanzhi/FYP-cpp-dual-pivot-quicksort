#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <random>

// Create a minimal dual-pivot quicksort implementation just for benchmarking
namespace minimal_dual_pivot {

template<typename T>
void insertionSort(T* a, int low, int high) {
    for (int i, k = low; ++k < high; ) {
        T ai = a[i = k];
        if (ai < a[i - 1]) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

template<typename T>
std::pair<int, int> partitionDualPivot(T* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    
    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    T pivot1 = a[e1];
    T pivot2 = a[e5];
    
    a[e1] = a[lower];
    a[e5] = a[upper];
    
    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);
    
    --lower;
    for (int k = ++upper; --k > lower; ) {
        T ak = a[k];
        
        if (ak < pivot1) {
            while (lower < k) {
                if (a[++lower] >= pivot1) {
                    if (a[lower] > pivot2) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (ak > pivot2) {
            a[k] = a[--upper];
            a[upper] = ak;
        }
    }
    
    a[low] = a[lower]; 
    a[lower] = pivot1;
    a[end] = a[upper]; 
    a[upper] = pivot2;
    
    return std::make_pair(lower, upper);
}

template<typename T>
void sort5Network(T* a, int e1, int e2, int e3, int e4, int e5) {
    auto conditional_swap = [](T& x, T& y) {
        if (y < x) {
            T temp = x;
            x = y;
            y = temp;
        }
    };
    
    conditional_swap(a[e5], a[e2]);
    conditional_swap(a[e4], a[e1]);
    conditional_swap(a[e5], a[e4]);
    conditional_swap(a[e2], a[e1]);
    conditional_swap(a[e4], a[e2]);
    
    T a3 = a[e3];
    if (a3 < a[e2]) {
        if (a3 < a[e1]) {
            a[e3] = a[e2]; a[e2] = a[e1]; a[e1] = a3;
        } else {
            a[e3] = a[e2]; a[e2] = a3;
        }
    } else if (a3 > a[e4]) {
        if (a3 > a[e5]) {
            a[e3] = a[e4]; a[e4] = a[e5]; a[e5] = a3;
        } else {
            a[e3] = a[e4]; a[e4] = a3;
        }
    }
}

template<typename T>
void sort(T* a, int low, int high) {
    static constexpr int MAX_INSERTION_SORT_SIZE = 44;
    static constexpr int DELTA = 6;
    static constexpr int MAX_RECURSION_DEPTH = 64 * DELTA;
    
    int bits = 0;
    while (true) {
        int end = high - 1;
        int size = high - low;
        
        if (size < MAX_INSERTION_SORT_SIZE) {
            insertionSort(a, low, high);
            return;
        }
        
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            std::sort(a + low, a + high);  // Fallback to std::sort
            return;
        }
        
        int step = (size >> 3) * 3 + 3;
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;
        
        sort5Network(a, e1, e2, e3, e4, e5);
        
        int lower, upper;
        
        if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
            auto pivotIndices = partitionDualPivot(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            sort(a, lower + 1, upper);
            sort(a, upper + 1, high);
        } else {
            // Single pivot fallback
            std::sort(a + low, a + high);
            return;
        }
        high = lower;
    }
}

template<typename RandomAccessIterator>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last) {
    if (first >= last) return;
    int size = last - first;
    if (size <= 1) return;
    
    auto* a = &(*first);
    sort(a, 0, size);
}

} // namespace minimal_dual_pivot

class SimpleBenchmark {
private:
    std::mt19937 gen{42};
    
public:
    std::vector<int> generateRandomData(size_t size) {
        std::vector<int> data;
        data.reserve(size);
        std::uniform_int_distribution<int> dis(1, static_cast<int>(size));
        
        for (size_t i = 0; i < size; ++i) {
            data.push_back(dis(gen));
        }
        return data;
    }
    
    template<typename SortFunc>
    double timeSort(const std::vector<int>& original_data, SortFunc sort_func) {
        std::vector<int> data = original_data;
        
        auto start = std::chrono::high_resolution_clock::now();
        sort_func(data);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        return duration.count() / 1e6;
    }
    
    void runBenchmark() {
        std::cout << "Running Simple Dual-Pivot Quicksort Benchmark\n";
        std::cout << "==============================================\n\n";
        
        std::ofstream results("benchmark_results.csv");
        results << "Size,Algorithm,Time_ms\n";
        
        std::vector<size_t> test_sizes = {100, 1000, 10000, 50000};
        
        for (size_t size : test_sizes) {
            std::cout << "Testing size: " << size << std::endl;
            
            auto data = generateRandomData(size);
            
            double std_sort_time = timeSort(data, [](std::vector<int>& arr) {
                std::sort(arr.begin(), arr.end());
            });
            
            double dual_pivot_time = timeSort(data, [](std::vector<int>& arr) {
                minimal_dual_pivot::dual_pivot_quicksort(arr.begin(), arr.end());
            });
            
            results << size << ",std::sort," << std::fixed << std::setprecision(3) << std_sort_time << "\n";
            results << size << ",dual_pivot_quicksort," << std::fixed << std::setprecision(3) << dual_pivot_time << "\n";
            
            std::cout << "  std::sort: " << std_sort_time << " ms\n";
            std::cout << "  dual_pivot: " << dual_pivot_time << " ms\n";
            std::cout << "  Speedup: " << (std_sort_time / dual_pivot_time) << "x\n\n";
        }
        
        results.close();
        std::cout << "Benchmark completed. Results saved to benchmark_results.csv\n";
    }
};

int main() {
    try {
        SimpleBenchmark benchmark;
        benchmark.runBenchmark();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}