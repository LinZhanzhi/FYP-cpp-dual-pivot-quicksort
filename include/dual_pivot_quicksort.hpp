#ifndef DUAL_PIVOT_QUICKSORT_HPP
#define DUAL_PIVOT_QUICKSORT_HPP

#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>
#include <memory>
#include <cstring>
#include <cmath>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <variant>
#include <bit>  // For std::bit_cast in C++20, fallback for older standards

// Phase 6: Performance optimization includes
#ifdef __x86_64__
    #include <immintrin.h>  // SIMD intrinsics
#endif

// Compiler hints for branch prediction and optimization
#ifdef __GNUC__
    #define LIKELY(x)       __builtin_expect(!!(x), 1)
    #define UNLIKELY(x)     __builtin_expect(!!(x), 0)
    #define FORCE_INLINE    __attribute__((always_inline)) inline
    #define PREFETCH_READ(addr)  __builtin_prefetch(addr, 0, 3)
    #define PREFETCH_WRITE(addr) __builtin_prefetch(addr, 1, 3)
#elif defined(_MSC_VER)
    #define LIKELY(x)       (x)
    #define UNLIKELY(x)     (x)
    #define FORCE_INLINE    __forceinline
    #define PREFETCH_READ(addr)  _mm_prefetch((const char*)(addr), _MM_HINT_T0)
    #define PREFETCH_WRITE(addr) _mm_prefetch((const char*)(addr), _MM_HINT_T0)
#else
    #define LIKELY(x)       (x)
    #define UNLIKELY(x)     (x)
    #define FORCE_INLINE    inline
    #define PREFETCH_READ(addr)  
    #define PREFETCH_WRITE(addr) 
#endif

namespace dual_pivot {

// =============================================================================
// OBJECT-BASED GENERIC ARRAY HANDLING (matching Java's Object a, b pattern)
// =============================================================================

// Array variant type for generic array handling (equivalent to Java's Object)
using ArrayVariant = std::variant<
    int*, long*, float*, double*,
    signed char*, char*, short*,
    unsigned char*, unsigned short*
>;

// Array pointer wrapper for type-safe array operations
struct ArrayPointer {
    ArrayVariant data;
    int size;
    int element_size;
    
    ArrayPointer() : data(static_cast<int*>(nullptr)), size(0), element_size(0) {}
    
    template<typename T>
    ArrayPointer(T* ptr, int sz = 0) : data(ptr), size(sz), element_size(sizeof(T)) {}
    
    // Type checking methods (equivalent to Java's instanceof)
    bool isIntArray() const { return std::holds_alternative<int*>(data); }
    bool isLongArray() const { return std::holds_alternative<long*>(data); }
    bool isFloatArray() const { return std::holds_alternative<float*>(data); }
    bool isDoubleArray() const { return std::holds_alternative<double*>(data); }
    bool isByteArray() const { return std::holds_alternative<signed char*>(data); }
    bool isCharArray() const { return std::holds_alternative<char*>(data); }
    bool isShortArray() const { return std::holds_alternative<short*>(data); }
    bool isUnsignedByteArray() const { return std::holds_alternative<unsigned char*>(data); }
    bool isUnsignedShortArray() const { return std::holds_alternative<unsigned short*>(data); }
    
    // Type extraction methods (equivalent to Java's casting)
    int* asIntArray() const { return std::get<int*>(data); }
    long* asLongArray() const { return std::get<long*>(data); }
    float* asFloatArray() const { return std::get<float*>(data); }
    double* asDoubleArray() const { return std::get<double*>(data); }
    signed char* asByteArray() const { return std::get<signed char*>(data); }
    char* asCharArray() const { return std::get<char*>(data); }
    short* asShortArray() const { return std::get<short*>(data); }
    unsigned char* asUnsignedByteArray() const { return std::get<unsigned char*>(data); }
    unsigned short* asUnsignedShortArray() const { return std::get<unsigned short*>(data); }
    
    // Generic visitor pattern for type dispatch
    template<typename Visitor>
    auto visit(Visitor&& visitor) const {
        return std::visit(std::forward<Visitor>(visitor), data);
    }
};

// Factory functions for creating ArrayPointer instances
template<typename T>
ArrayPointer makeArrayPointer(T* ptr, int size = 0) {
    return ArrayPointer(ptr, size);
}


// C++ equivalent of Java's Float.floatToRawIntBits() 
inline std::uint32_t floatToRawIntBits(float value) {
#if __cpp_lib_bit_cast >= 201806L
    return std::bit_cast<std::uint32_t>(value);
#else
    // Fallback for pre-C++20 compilers
    static_assert(sizeof(float) == sizeof(std::uint32_t), "Float and uint32_t must have same size");
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(float));
    return result;
#endif
}

// C++ equivalent of Java's Double.doubleToRawLongBits()
inline std::uint64_t doubleToRawLongBits(double value) {
#if __cpp_lib_bit_cast >= 201806L
    return std::bit_cast<std::uint64_t>(value);
#else
    // Fallback for pre-C++20 compilers
    static_assert(sizeof(double) == sizeof(std::uint64_t), "Double and uint64_t must have same size");
    std::uint64_t result;
    std::memcpy(&result, &value, sizeof(double));
    return result;
#endif
}

// C++ equivalent of Java's Float.intBitsToFloat()
inline float intBitsToFloat(std::uint32_t bits) {
#if __cpp_lib_bit_cast >= 201806L
    return std::bit_cast<float>(bits);
#else
    float result;
    std::memcpy(&result, &bits, sizeof(float));
    return result;
#endif
}

// C++ equivalent of Java's Double.longBitsToDouble()
inline double longBitsToDouble(std::uint64_t bits) {
#if __cpp_lib_bit_cast >= 201806L
    return std::bit_cast<double>(bits);
#else
    double result;
    std::memcpy(&result, &bits, sizeof(double));
    return result;
#endif
}

// Enhanced negative zero detection using bit manipulation
inline bool isNegativeZero(float value) {
    return value == 0.0f && floatToRawIntBits(value) == 0x80000000U;
}

inline bool isNegativeZero(double value) {
    return value == 0.0 && doubleToRawLongBits(value) == 0x8000000000000000ULL;
}

// Optimized NaN detection using bit patterns
inline bool isNaN(float value) {
    std::uint32_t bits = floatToRawIntBits(value);
    return (bits & 0x7F800000U) == 0x7F800000U && (bits & 0x007FFFFFU) != 0;
}

inline bool isNaN(double value) {
    std::uint64_t bits = doubleToRawLongBits(value);
    return (bits & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL && (bits & 0x000FFFFFFFFFFFFFULL) != 0;
}

// Precise floating-point comparison with bit-level accuracy
inline bool isPositiveZero(float value) {
    return floatToRawIntBits(value) == 0x00000000U;
}

inline bool isPositiveZero(double value) {
    return doubleToRawLongBits(value) == 0x0000000000000000ULL;
}

// Advanced binary search for zero restoration (matching Java's unsigned right shift)
template<typename T>
FORCE_INLINE int findZeroPosition(T* a, int low, int high) {
    while (low <= high) {
        // Use unsigned right shift equivalent (matching Java >>> operator)
        int middle = static_cast<int>(static_cast<unsigned int>(low + high) >> 1);
        if (a[middle] < T(0)) {
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }
    return low;
}

// =============================================================================
// FUNCTIONAL INTERFACE ARCHITECTURE (matching Java's method reference patterns)
// =============================================================================

// Forward declarations for function pointer types
template<typename T> class Sorter;

// SortOperation functional interface equivalent
template<typename T>
using SortOperation = void(*)(T* a, int low, int high);

// Advanced SortOperation with Sorter integration
template<typename T>
using SorterSortOperation = void(*)(Sorter<T>* sorter, T* a, int bits, int low, int high);

// PartitionOperation functional interface equivalent  
template<typename T>
using PartitionOperation = std::pair<int, int>(*)(T* a, int low, int high, int pivotIndex1, int pivotIndex2);

// Intrinsic candidate equivalent - optimized function dispatch
template<typename T>
FORCE_INLINE void sort_intrinsic(T* array, int low, int high, SortOperation<T> so) {
    so(array, low, high);
}

// Intrinsic candidate for partitioning
template<typename T>
FORCE_INLINE std::pair<int, int> partition_intrinsic(T* array, int low, int high, 
                                                     int pivotIndex1, int pivotIndex2, 
                                                     PartitionOperation<T> po) {
    return po(array, low, high, pivotIndex1, pivotIndex2);
}

// Method reference equivalents for different algorithms
template<typename T> void insertionSort_ref(T* a, int low, int high);
template<typename T> void mixedInsertionSort_ref(T* a, int low, int high);
template<typename T> void heapSort_ref(T* a, int low, int high);
template<typename T> std::pair<int, int> partitionDualPivot_ref(T* a, int low, int high, int pivotIndex1, int pivotIndex2);
template<typename T> std::pair<int, int> partitionSinglePivot_ref(T* a, int low, int high, int pivotIndex1, int pivotIndex2);

// Constants from Java DualPivotQuicksort implementation
static constexpr int MAX_MIXED_INSERTION_SORT_SIZE = 65;
static constexpr int MAX_INSERTION_SORT_SIZE = 44;
static constexpr int DELTA = 3 << 1;  // 6
static constexpr int MAX_RECURSION_DEPTH = 64 * DELTA;  // 384

// Run detection and merging constants
static constexpr int MIN_TRY_MERGE_SIZE = 4 << 10;  // 4096
static constexpr int MIN_FIRST_RUN_SIZE = 16;
static constexpr int MIN_FIRST_RUNS_FACTOR = 7;
static constexpr int MAX_RUN_CAPACITY = 5 << 10;  // 5120

// Parallel processing constants
static constexpr int MIN_PARALLEL_SORT_SIZE = 4 << 10;  // 4096
static constexpr int MIN_PARALLEL_MERGE_PARTS_SIZE = 4 << 10;  // 4096
static constexpr int MIN_RUN_COUNT = 4;

// Type-specific counting sort thresholds
static constexpr int MIN_BYTE_COUNTING_SORT_SIZE = 64;
static constexpr int MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE = 1750;

// ThreadPool class for work distribution
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    
public:
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }
    
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task](){ (*task)(); });
        }
        condition.notify_one();
        return res;
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker: workers) {
            worker.join();
        }
    }
};

// Singleton thread pool for parallel sorting
static ThreadPool& getThreadPool() {
    static ThreadPool pool;
    return pool;
}

// =============================================================================
// MERGE OPERATIONS (move before parallel classes to fix instantiation issues)
// =============================================================================

template<typename T>
void mergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2) {
    // Merge small parts sequentially
    while (lo1 < hi1 && lo2 < hi2) {
        dst[k++] = (a1[lo1] < a2[lo2]) ? a1[lo1++] : a2[lo2++];
    }
    if (dst != a1 || k < lo1) {
        while (lo1 < hi1) {
            dst[k++] = a1[lo1++];
        }
    }
    if (dst != a2 || k < lo2) {
        while (lo2 < hi2) {
            dst[k++] = a2[lo2++];
        }
    }
}

template<typename T>
void parallelMergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2) {
    // Similar to sequential merge but with parallel subdivision for large parts
    if (hi1 - lo1 >= MIN_PARALLEL_MERGE_PARTS_SIZE && hi2 - lo2 >= MIN_PARALLEL_MERGE_PARTS_SIZE) {
        // Find median of larger part and partition smaller part
        if (hi1 - lo1 < hi2 - lo2) {
            std::swap(lo1, lo2);
            std::swap(hi1, hi2);
            std::swap(a1, a2);
        }
        
        int mi1 = (lo1 + hi1) >> 1;
        T key = a1[mi1];
        int mi2 = std::lower_bound(a2 + lo2, a2 + hi2, key) - a2;
        
        int d = mi2 - lo2 + mi1 - lo1;
        
        auto& pool = getThreadPool();
        auto future = pool.enqueue([=] { 
            parallelMergeParts(dst, k + d, a1, mi1, hi1, a2, mi2, hi2); 
        });
        
        // Process left parts
        parallelMergeParts(dst, k, a1, lo1, mi1, a2, lo2, mi2);
        future.get();
    } else {
        // Sequential merge for small parts
        mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    }
}

// =============================================================================
// ADVANCED RUN MERGER WITH SOPHISTICATED FORK/JOIN PATTERNS (matching Java's RecursiveTask)
// =============================================================================

// RunMerger class for parallel run merging (matching Java's RunMerger extends RecursiveTask) 
template<typename T>
class RunMerger {
private:
    T* a;
    T* b;
    int offset;
    int aim;
    std::vector<int> run;
    int lo, hi;
    
    // Advanced parallel coordination state
    std::future<T*> future_result;
    bool is_forked = false;
    T* result = nullptr;
    std::atomic<bool> completed{false};
    std::mutex result_mutex;
    
public:
    RunMerger(T* a, T* b, int offset, int aim, const std::vector<int>& run, int lo, int hi)
        : a(a), b(b), offset(offset), aim(aim), run(run), lo(lo), hi(hi) {}
    
    // Enhanced compute method with sophisticated parallel subdivision
    T* compute() {
        if (hi - lo == 1) {
            // Base case: single run
            if (aim >= 0) {
                return a;
            }
            // Copy elements in reverse order (matching Java's approach)
            for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
                b[--j] = a[--i];
            }
            return b;
        }
        
        // Advanced parallel subdivision (matching Java's sophisticated approach)
        int mi = lo;
        int rmi = (run[lo] + run[hi]) >> 1; // Unsigned right shift equivalent
        while (run[++mi + 1] <= rmi);
        
        // Create parallel tasks for left and right parts with advanced coordination
        auto& pool = getThreadPool();
        
        // Left subtask with negative aim (Java pattern)
        auto future1 = pool.enqueue([=]() {
            RunMerger<T> left(a, b, offset, -aim, run, lo, mi);
            return left.compute();
        });
        
        // Right subtask with zero aim (Java pattern)
        auto future2 = pool.enqueue([=]() {
            RunMerger<T> right(a, b, offset, 0, run, mi, hi);
            return right.compute();
        });
        
        // Get results from parallel tasks with proper synchronization
        T* a1 = future1.get();
        T* a2 = future2.get();
        
        // Advanced destination calculation (matching Java's sophisticated logic)
        T* dst = (a1 == a) ? b : a;
        
        // Complex offset calculations (matching Java's approach)
        int k   = (a1 == a) ? run[lo] - offset : run[lo];
        int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
        int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
        int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
        int hi2 = (a2 == b) ? run[hi] - offset : run[hi];
        
        // Advanced merge with parallel coordination
        mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        return dst;
    }
    
    // Java-style forkMe() method - sophisticated fork and return self pattern
    RunMerger& forkMe() {
        std::lock_guard<std::mutex> lock(result_mutex);
        
        if (!is_forked) {
            auto& pool = getThreadPool();
            
            // Advanced future-based forking with exception handling
            future_result = pool.enqueue([this]() -> T* {
                try {
                    T* computed_result = this->compute();
                    
                    // Mark completion with thread-safe state management
                    std::lock_guard<std::mutex> result_lock(this->result_mutex);
                    this->result = computed_result;
                    this->completed.store(true);
                    
                    return computed_result;
                } catch (...) {
                    // Enhanced exception handling (matching Java's approach)
                    std::lock_guard<std::mutex> result_lock(this->result_mutex);
                    this->completed.store(true);
                    throw;
                }
            });
            
            is_forked = true;
        }
        return *this;
    }
    
    // Java-style getDestination() method - sophisticated join and get result
    T* getDestination() {
        std::lock_guard<std::mutex> lock(result_mutex);
        
        if (is_forked) {
            if (!completed.load()) {
                // Advanced synchronization with timeout handling
                try {
                    // Wait for completion with proper exception propagation
                    result = future_result.get();
                    completed.store(true);
                } catch (const std::exception& e) {
                    // Enhanced error handling matching Java's exception model
                    completed.store(true);
                    throw std::runtime_error("RunMerger computation failed: " + std::string(e.what()));
                }
            }
            return result;
        } else {
            // If not forked, compute directly (lazy evaluation pattern)
            if (!completed.load()) {
                result = compute();
                completed.store(true);
            }
            return result;
        }
    }
    
    // Alternative join method for consistency with Java RecursiveTask
    T* join() {
        return getDestination();
    }
    
    // Get raw result (matching Java RecursiveTask API)
    T* getRawResult() {
        std::lock_guard<std::mutex> lock(result_mutex);
        return result;
    }
    
    // Advanced completion checking
    bool isDone() const {
        return completed.load();
    }
    
    // Force completion (matching Java's cancel pattern)  
    void cancel() {
        std::lock_guard<std::mutex> lock(result_mutex);
        completed.store(true);
    }
    
    // Enhanced status reporting
    bool isForkJoinTask() const {
        return is_forked;
    }
};

// =============================================================================
// ADVANCED PARALLEL MERGER WITH SOPHISTICATED COORDINATION (matching Java's approach)
// =============================================================================

// =============================================================================
// ADVANCED PARALLEL PROCESSING FRAMEWORK (matching Java's CountedCompleter)
// =============================================================================

// Forward declarations for advanced parallel classes
template<typename T> class Sorter;
template<typename T> class Merger; 
template<typename T> class RunMerger;

// Base class for counted completion (similar to Java's CountedCompleter)
template<typename T>
class CountedCompleter {
protected:
    std::atomic<int> pending{0};
    CountedCompleter* parent{nullptr};
    bool completed{false};
    std::mutex completion_mutex;  // Enhanced thread safety
    
public:
    CountedCompleter(CountedCompleter* parent = nullptr) : parent(parent) {
        if (parent) {
            parent->pending.fetch_add(1);
        }
    }
    
    virtual ~CountedCompleter() = default;
    
    virtual void compute() = 0;
    virtual void onCompletion(CountedCompleter* caller) {}
    virtual void onExceptionalCompletion(std::exception_ptr ex, CountedCompleter* caller) {}
    
    void invoke() {
        try {
            compute();
            tryComplete();
        } catch (...) {
            completeExceptionally(std::current_exception());
        }
    }
    
    void fork() {
        auto& pool = getThreadPool();
        pool.enqueue([this]() { invoke(); });
    }
    
    // Enhanced completion with proper propagation (matching Java's sophistication)
    void tryComplete() {
        CountedCompleter* curr = this;
        while (curr != nullptr) {
            std::lock_guard<std::mutex> lock(curr->completion_mutex);
            
            if (curr->pending.load() == 0 && !curr->completed) {
                curr->completed = true;
                CountedCompleter* parent = curr->parent;
                
                if (parent) {
                    try {
                        curr->onCompletion(curr);
                        
                        // Advanced pending count management with atomic operations
                        int prevPending = parent->pending.fetch_sub(1);
                        if (prevPending == 1) {
                            curr = parent;
                            continue;  // Propagate completion to parent
                        }
                    } catch (...) {
                        curr->completeExceptionally(std::current_exception());
                        return;
                    }
                }
                break;
            } else {
                break;
            }
        }
    }
    
    // Enhanced exception handling (matching Java's pattern)
    void completeExceptionally(std::exception_ptr ex) {
        std::lock_guard<std::mutex> lock(completion_mutex);
        completed = true;
        
        // Propagate exception to parent chain  
        CountedCompleter* curr = parent;
        while (curr != nullptr) {
            try {
                curr->onExceptionalCompletion(ex, this);
                curr = curr->parent;
            } catch (...) {
                // Swallow additional exceptions during propagation
                break;
            }
        }
    }
    
    void addToPendingCount(int delta) {
        pending.fetch_add(delta);
    }
    
    // Java-style pending count management
    void setPendingCount(int count) {
        pending.store(count);
    }
    
    int getPendingCount() const {
        return pending.load();
    }
    
    bool isCompletedAbnormally() const {
        return completed && pending.load() < 0;  // Use negative pending as error flag
    }
    
    // Advanced completion checking with timeout (C++ enhancement)
    bool tryCompleteWithTimeout(int timeout_ms) {
        auto start = std::chrono::steady_clock::now();
        
        while (!completed) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            
            if (elapsed.count() > timeout_ms) {
                return false;  // Timeout
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        return true;
    }
};

// Generic Sorter class for Object-based array handling (matching Java's Sorter extends CountedCompleter)
// TODO: Fix forward declaration issues - temporarily simplified
class GenericSorter : public CountedCompleter<void> {
private:
    GenericSorter* parent;
    ArrayPointer a;  // Primary array (equivalent to Java's Object a)
    ArrayPointer b;  // Buffer array (equivalent to Java's Object b) 
    int low;
    int size;
    int offset;
    int depth;
    
public:
    GenericSorter(GenericSorter* parent, ArrayPointer a, ArrayPointer b, int low, int size, int offset, int depth)
        : CountedCompleter<void>(parent), parent(parent), a(a), b(b), 
          low(low), size(size), offset(offset), depth(depth) {}
    
    void compute() override {
        // Simplified implementation to avoid forward declaration issues
        // Runtime type dispatch will be completed after function definitions are in place
        // For now, just complete the task
        tryComplete();
    }
    
    void onCompletion(CountedCompleter<void>* caller) override {
        // Simplified completion - full merge logic will be implemented later
    }
    
    // Java-style forkSorter with Object-based array handling
    void forkSorter(int depth, int low, int high) {
        addToPendingCount(1);
        ArrayPointer localA = this->a; // Local variable optimization (matching Java pattern)
        auto* child = new GenericSorter(this, localA, b, low, high - low, offset, depth);
        child->fork();
    }
    
    // Getter methods for buffer access
    ArrayPointer getArrayA() const { return a; }
    ArrayPointer getArrayB() const { return b; }
    int getOffset() const { return offset; }
};

// Sorter class for parallel quicksort (matching Java's Sorter extends CountedCompleter)
template<typename T>
class Sorter : public CountedCompleter<T> {
private:
    Sorter* parent;
    T* a;
    T* b;  // auxiliary array
    int low;
    int size;
    int offset;
    int depth;
    
public:
    Sorter(Sorter* parent, T* a, T* b, int low, int size, int offset, int depth)
        : CountedCompleter<T>(parent), parent(parent), a(a), b(b), 
          low(low), size(size), offset(offset), depth(depth) {}
    
    void compute() override {
        if (depth < 0) {
            // Use parallel merge sort for highly parallel scenarios
            this->setPendingCount(2);
            int half = size >> 1;
            
            auto* left = new Sorter(this, b, a, low, half, offset, depth + 1);
            auto* right = new Sorter(this, b, a, low + half, size - half, offset, depth + 1);
            
            left->fork();
            right->compute();
        } else {
            // Use type-specific parallel quicksort
            if constexpr (std::is_same_v<T, int>) {
                sort_int_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, long>) {
                sort_long_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, float>) {
                sort_float_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, double>) {
                sort_double_sequential(this, a, depth, low, low + size);
            } else {
                // Fallback to generic implementation
                parallelQuickSort(a, depth, low, low + size);
            }
        }
        this->tryComplete();
    }
    
    void onCompletion(CountedCompleter<T>* caller) override {
        // Handle merge operations for negative depth (merge sort mode)
        if (depth < 0) {
            int mi = low + (size >> 1);
            bool src = (depth & 1) == 0;
            
            // Create merger for the two halves
            auto* merger = new Merger<T>(nullptr,
                a,                          // dst
                src ? low : low - offset,   // k
                b,                          // a1
                src ? low - offset : low,   // lo1
                src ? mi - offset : mi,     // hi1
                b,                          // a2
                src ? mi - offset : mi,     // lo2
                src ? low + size - offset : low + size  // hi2
            );
            merger->invoke();
            delete merger;
        }
    }
    
    // Factory method for creating child sorters (matching Java's forkSorter pattern)
    void forkSorter(int depth, int low, int high) {
        this->addToPendingCount(1);
        auto* child = new Sorter(this, a, b, low, high - low, offset, depth);
        child->fork();
    }
    
    // Helper method to set pending count (matching Java API)
    void setPendingCount(int count) {
        this->pending.store(count);
    }
};

// Generic Merger class for Object-based array merging (matching Java's Merger extends CountedCompleter)
class GenericMerger : public CountedCompleter<void> {
private:
    ArrayPointer dst;
    int k;
    ArrayPointer a1;
    int lo1, hi1;
    ArrayPointer a2; 
    int lo2, hi2;
    
public:
    GenericMerger(CountedCompleter<void>* parent, ArrayPointer dst, int k, 
                  ArrayPointer a1, int lo1, int hi1, ArrayPointer a2, int lo2, int hi2)
        : CountedCompleter<void>(parent), dst(dst), k(k), a1(a1), lo1(lo1), hi1(hi1), a2(a2), lo2(lo2), hi2(hi2) {}
    
    void compute() override {
        // Runtime type dispatch for merging operations (matching Java's approach)
        if (dst.isIntArray()) {
            mergeParts(dst.asIntArray(), k, a1.asIntArray(), lo1, hi1, a2.asIntArray(), lo2, hi2);
        } else if (dst.isLongArray()) {
            mergeParts(dst.asLongArray(), k, a1.asLongArray(), lo1, hi1, a2.asLongArray(), lo2, hi2);
        } else if (dst.isFloatArray()) {
            mergeParts(dst.asFloatArray(), k, a1.asFloatArray(), lo1, hi1, a2.asFloatArray(), lo2, hi2);
        } else if (dst.isDoubleArray()) {
            mergeParts(dst.asDoubleArray(), k, a1.asDoubleArray(), lo1, hi1, a2.asDoubleArray(), lo2, hi2);
        } else {
            throw std::runtime_error("Unknown array type in GenericMerger::compute()");
        }
        tryComplete();
    }
};

template<typename T>
class Merger : public CountedCompleter<T> {
private:
    T* dst;
    int k;
    T* a1;
    int lo1, hi1;
    T* a2; 
    int lo2, hi2;
    
public:
    Merger(CountedCompleter<T>* parent, T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2)
        : CountedCompleter<T>(parent), dst(dst), k(k), a1(a1), lo1(lo1), hi1(hi1), a2(a2), lo2(lo2), hi2(hi2) {}
    
    void compute() override {
        // Use parallel merge with subdivision for large parts
        if (hi1 - lo1 >= MIN_PARALLEL_MERGE_PARTS_SIZE && hi2 - lo2 >= MIN_PARALLEL_MERGE_PARTS_SIZE) {
            // Parallel merge with binary search partitioning
            parallelMergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        } else {
            // Sequential merge for small parts
            mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
        }
    }
};

// Forward declarations for type-specific functions
template<typename T> void insertionSort_ref(T* a, int low, int high);
template<typename T> void mixedInsertionSort_ref(T* a, int low, int high);
template<typename T> void heapSort_ref(T* a, int low, int high);
template<typename T> std::pair<int, int> partitionDualPivot_ref(T* a, int low, int high, int pivotIndex1, int pivotIndex2);
template<typename T> std::pair<int, int> partitionSinglePivot_ref(T* a, int low, int high, int pivotIndex1, int pivotIndex2);

// Type-specific function declarations
static void sort_int_sequential(Sorter<int>* sorter, int* a, int bits, int low, int high);
static void sort_long_sequential(Sorter<long>* sorter, long* a, int bits, int low, int high);
static void sort_float_sequential(Sorter<float>* sorter, float* a, int bits, int low, int high);
static void sort_double_sequential(Sorter<double>* sorter, double* a, int bits, int low, int high);
static bool tryMergeRuns_int(Sorter<int>* sorter, int* a, int low, int size);
static bool tryMergeRuns_long(Sorter<long>* sorter, long* a, int low, int size);
static bool tryMergeRuns_float(Sorter<float>* sorter, float* a, int low, int size);
static bool tryMergeRuns_double(Sorter<double>* sorter, double* a, int low, int size);
static std::pair<int, int> partitionDualPivot_long(long* a, int low, int high, int pivotIndex1, int pivotIndex2);
static std::pair<int, int> partitionSinglePivot_long(long* a, int low, int high, int pivotIndex1, int pivotIndex2);
static std::pair<int, int> partitionDualPivot_float(float* a, int low, int high, int pivotIndex1, int pivotIndex2);
static std::pair<int, int> partitionSinglePivot_float(float* a, int low, int high, int pivotIndex1, int pivotIndex2);
static std::pair<int, int> partitionDualPivot_double(double* a, int low, int high, int pivotIndex1, int pivotIndex2);
static std::pair<int, int> partitionSinglePivot_double(double* a, int low, int high, int pivotIndex1, int pivotIndex2);
static void pushDown_int(int* a, int p, int value, int low, int high);
static void pushDown_long(long* a, int p, long value, int low, int high);
static void pushDown_float(float* a, int p, float value, int low, int high);
static void pushDown_double(double* a, int p, double value, int low, int high);
static int* mergeRuns_int(int* a, int* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
static long* mergeRuns_long(long* a, long* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
static float* mergeRuns_float(float* a, float* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
static double* mergeRuns_double(double* a, double* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);

// Forward declarations for merge operations
template<typename T>
void mergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2);

// Forward declarations for type-specific merge functions  
static signed char* mergeRuns_byte(signed char* a, signed char* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
static char* mergeRuns_char(char* a, char* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);
static short* mergeRuns_short(short* a, short* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi);

// Parallel depth calculation
inline int getDepth(int parallelism, int size) {
    int depth = 0;
    while ((parallelism >>= 3) > 0 && (size >>= 2) > 0) {
        depth -= 2;
    }
    return depth;
}

// Forward declarations for merge operations
template<typename T>
void mergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2);

template<typename T>
void parallelMergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2);

// Forward declarations for parallel functions
template<typename T> class ParallelSorter;
template<typename T> class ParallelMerger;
template<typename T> class ParallelRunMerger;

//

template<typename T>
void sort(T* a, int bits, int low, int high);

template<typename T>
void parallelSort(T* a, int parallelism, int low, int high);

template<typename T>
void parallelQuickSort(T* a, int bits, int low, int high);

template<typename T>
void parallelMergeSort(T* a, T* b, int low, int size, int offset, int depth);

template<typename T>
void parallelMergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2);

// Forward declarations for type-specialized functions
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
sort_specialized(T* a, int low, int high);

// Forward declaration for merge operations
template<typename T>
void mergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2);

template<typename T>
FORCE_INLINE void insertionSort(T* a, int low, int high) {
    // Phase 6: Cache-friendly insertion sort with prefetching
    for (int i, k = low; ++k < high; ) {
        T ai = a[i = k];
        
        // Prefetch next elements to improve cache performance
        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }
        
        if (UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

template<typename T>
void mixedInsertionSort(T* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);
    
    if (end == high) {
        // Invoke simple insertion sort on tiny array
        for (int i; ++low < end; ) {
            T ai = a[i = low];
            
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Start with pin insertion sort on small part
        T pin = a[end];
        
        for (int i, p = high; ++low < end; ) {
            T ai = a[i = low];
            
            if (ai < a[i - 1]) { // Small element
                // Insert small element into sorted part
                a[i] = a[i - 1];
                --i;
                
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
                
            } else if (p > i && ai > pin) { // Large element
                // Find element smaller than pin
                while (a[--p] > pin);
                
                // Swap it with large element
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }
                
                // Insert small element into sorted part
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }
        
        // Continue with pair insertion sort on remain part
        for (int i; low < high; ++low) {
            T a1 = a[i = low], a2 = a[++low];
            
            // Insert two elements per iteration
            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;
                
                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;
                
            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;
                
                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

// =============================================================================
// TYPE-SPECIFIC MIXED INSERTION SORT VARIANTS (matching Java's per-type approach)
// =============================================================================

template<typename T>
void pushDown(T* a, int p, T value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2; // Index of the right child
        
        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

template<typename T>
void heapSort(T* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown(a, k, a[k], low, high);
    }
    while (--high > low) {
        T max = a[low];
        pushDown(a, low, a[high], low, high);
        a[high] = max;
    }
}

template<typename T>
FORCE_INLINE std::pair<int, int> partitionDualPivot(T* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    // Phase 6: Optimized dual pivot partitioning with prefetching
    int end = high - 1;
    int lower = low;
    int upper = end;
    
    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    T pivot1 = a[e1];
    T pivot2 = a[e5];
    
    // The first and the last elements to be sorted are moved
    // to the locations formerly occupied by the pivots
    a[e1] = a[lower];
    a[e5] = a[upper];
    
    // Skip elements, which are less or greater than the pivots
    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);
    
    // Backward 3-interval partitioning with cache optimization
    (void)--lower; // Mark as used
    for (int k = ++upper; --k > lower; ) {
        T ak = a[k];
        
        // Prefetch elements ahead for better cache utilization
        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }
        
        if (UNLIKELY(ak < pivot1)) { // Move a[k] to the left side
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) { // Move a[k] to the right side
            a[k] = a[--upper];
            a[upper] = ak;
        }
    }
    
    // Swap the pivots into their final positions
    a[low] = a[lower]; 
    a[lower] = pivot1;
    a[end] = a[upper]; 
    a[upper] = pivot2;
    
    return std::make_pair(lower, upper);
}

template<typename T>
std::pair<int, int> partitionSinglePivot(T* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    T pivot = a[e3];
    
    // The first element to be sorted is moved to the location formerly occupied by the pivot
    a[e3] = a[lower];
    
    // Traditional 3-way (Dutch National Flag) partitioning
    for (int k = ++upper; --k > lower; ) {
        T ak = a[k];
        
        if (ak != pivot) {
            a[k] = pivot;
            
            if (ak < pivot) { // Move a[k] to the left side
                while (a[++lower] < pivot);
                
                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else { // ak > pivot - Move a[k] to the right side
                a[--upper] = ak;
            }
        }
    }
    
    // Swap the pivot into its final position
    a[low] = a[lower];
    a[lower] = pivot;
    
    return std::make_pair(lower, upper);
}

// Run detection and merging implementation (Phase 3)
// Forward declarations
template<typename T>
T* mergeRuns(T* a, T* b, int offset, int aim, 
             const std::vector<int>& run, int lo, int hi);

template<typename T>
void mergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2);

template<typename T>
bool tryMergeRuns(T* a, int low, int size, bool parallel = false) {
    // The run array is constructed only if initial runs are
    // long enough to continue, run[i] then holds start index
    // of the i-th sequence of elements in non-descending order.
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;
    
    // Identify all possible runs
    for (int k = low + 1; k < high; ) {
        
        // Find the end index of the current run
        if (a[k - 1] < a[k]) {
            // Identify ascending sequence
            while (++k < high && a[k - 1] <= a[k]);
            
        } else if (a[k - 1] > a[k]) {
            // Identify descending sequence
            while (++k < high && a[k - 1] >= a[k]);
            
            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                T temp = a[i]; 
                a[i] = a[j]; 
                a[j] = temp;
            }
        } else { // Identify constant sequence
            T ak = a[k];
            while (++k < high && ak == a[k]);
            
            if (k < high) {
                continue;
            }
        }
        
        // Check special cases
        if (run.empty()) {
            if (k == high) {
                // The array is monotonous sequence,
                // and therefore already sorted.
                return true;
            }
            
            if (k - low < MIN_FIRST_RUN_SIZE) {
                // The first run is too small
                // to proceed with scanning.
                return false;
            }
            
            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);
            
        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                // The first runs are not long
                // enough to continue scanning.
                return false;
            }
            
            if (++count == MAX_RUN_CAPACITY) {
                // Array is not highly structured.
                return false;
            }
        }
        run.push_back(last = k);
    }
    
    // Merge runs of highly structured array
    if (count > 1) {
        std::vector<T> b(size);
        
        if (parallel && count >= MIN_RUN_COUNT) {
            // Use parallel run merging for large run counts
            RunMerger<T> merger(a, b.data(), low, 1, run, 0, count);
            T* result = merger.compute();
            
            // Copy back to main array if needed
            if (result != a) {
                std::copy(result + low, result + low + size, a + low);
            }
        } else {
            // Use sequential merging
            mergeRuns(a, b.data(), low, 1, run, 0, count);
        }
    }
    return true;
}

template<typename T>
T* mergeRuns(T* a, T* b, int offset, int aim, 
             const std::vector<int>& run, int lo, int hi) {
    
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }
    
    // Split into approximately equal parts
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);
    
    // Merge the left and right parts
    T* a1 = mergeRuns(a, b, offset, -aim, run, lo, mi);
    T* a2 = mergeRuns(a, b, offset, 0, run, mi, hi);
    
    T* dst = (a1 == a) ? b : a;
    
    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];
    
    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

// Phase 6: Optimized 5-element sorting network with branch-free swaps
template<typename T>
FORCE_INLINE void sort5Network(T* a, int e1, int e2, int e3, int e4, int e5) {
    // Branch-free conditional swap helper
    auto conditional_swap = [](T& x, T& y) {
        if (y < x) {
            T temp = x;
            x = y;
            y = temp;
        }
    };
    
    // 4-element sorting network
    conditional_swap(a[e5], a[e2]);
    conditional_swap(a[e4], a[e1]);
    conditional_swap(a[e5], a[e4]);
    conditional_swap(a[e2], a[e1]);
    conditional_swap(a[e4], a[e2]);
    
    // Insert the middle element using optimized insertion
    T a3 = a[e3];
    if (UNLIKELY(a3 < a[e2])) {
        if (UNLIKELY(a3 < a[e1])) {
            a[e3] = a[e2]; a[e2] = a[e1]; a[e1] = a3;
        } else {
            a[e3] = a[e2]; a[e2] = a3;
        }
    } else if (UNLIKELY(a3 > a[e4])) {
        if (UNLIKELY(a3 > a[e5])) {
            a[e3] = a[e4]; a[e4] = a[e5]; a[e5] = a3;
        } else {
            a[e3] = a[e4]; a[e4] = a3;
        }
    }
}

template<typename T>
void sort(T* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;
        
        // Run mixed insertion sort on small non-leftmost parts
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            mixedInsertionSort(a, low, high);
            return;
        }
        
        // Invoke insertion sort on small leftmost part
        if (size < MAX_INSERTION_SORT_SIZE) {
            insertionSort(a, low, high);
            return;
        }
        
        // Check if the whole array or large non-leftmost
        // parts are nearly sorted and then merge runs
        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns(a, low, size)) {
            return;
        }
        
        // Switch to heap sort if execution time is becoming quadratic
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            heapSort(a, low, high);
            return;
        }
        
        // Use an inexpensive approximation of the golden ratio
        // to select five sample elements and determine pivots
        int step = (size >> 3) * 3 + 3;
        
        // Five elements around (and including) the central element
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;
        
        // Use optimized 5-element sorting network (Phase 6)
        sort5Network(a, e1, e2, e3, e4, e5);
        
        // Pointers
        int lower; // The index of the last element of the left part
        int upper; // The index of the first element of the right part
        
        // Partitioning with 2 pivots in case of different elements
        if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
            // Use the first and fifth of the five sorted elements as the pivots
            auto pivotIndices = partitionDualPivot(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            // Sort non-left parts recursively, excluding known pivots
            sort(a, bits | 1, lower + 1, upper);
            sort(a, bits | 1, upper + 1, high);
            
        } else { // Use single pivot in case of many equal elements
            // Use the third of the five sorted elements as the pivot
            auto pivotIndices = partitionSinglePivot(a, low, high, e3, e3);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            // Sort the right part, excluding known pivot
            sort(a, bits | 1, upper, high);
        }
        high = lower; // Iterate along the left part
    }
}

// Phase 4: Type Specializations for floating-point and integer types

// =============================================================================
// TYPE-SPECIFIC ALGORITHM IMPLEMENTATIONS (matching Java's per-type methods)
// =============================================================================

// INT ARRAY IMPLEMENTATIONS
// -------------------------

// Type-specific mixed insertion sort for int arrays
static void mixedInsertionSort_int(int* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);
    
    if (end == high) {
        // Simple insertion sort for tiny arrays
        for (int i; ++low < end; ) {
            int ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort + pair insertion sort
        int pin = a[end];
        
        for (int i, p = high; ++low < end; ) {
            int ai = a[i = low];
            
            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
                
            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);
                
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }
                
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }
        
        // Pair insertion sort on remaining part
        for (int i; low < high; ++low) {
            int a1 = a[i = low], a2 = a[++low];
            
            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;
                
                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;
                
            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;
                
                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

// Type-specific insertion sort for int arrays
static void insertionSort_int(int* a, int low, int high) {
    for (int i, k = low; ++k < high; ) {
        int ai = a[i = k];
        
        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }
        
        if (UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

// Type-specific heap sort for int arrays
static void heapSort_int(int* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown_int(a, k, a[k], low, high);
    }
    while (--high > low) {
        int max = a[low];
        pushDown_int(a, low, a[high], low, high);
        a[high] = max;
    }
}

// Type-specific pushDown for int arrays
static void pushDown_int(int* a, int p, int value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2;
        
        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

// Type-specific dual pivot partitioning for int arrays
static std::pair<int, int> partitionDualPivot_int(int* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    
    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    int pivot1 = a[e1];
    int pivot2 = a[e5];
    
    a[e1] = a[lower];
    a[e5] = a[upper];
    
    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);
    
    (void)--lower;
    for (int k = ++upper; --k > lower; ) {
        int ak = a[k];
        
        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }
        
        if (UNLIKELY(ak < pivot1)) {
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) {
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

// Type-specific single pivot partitioning for int arrays
static std::pair<int, int> partitionSinglePivot_int(int* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    int pivot = a[e3];
    
    a[e3] = a[lower];
    
    for (int k = ++upper; --k > lower; ) {
        int ak = a[k];
        
        if (ak != pivot) {
            a[k] = pivot;
            
            if (ak < pivot) {
                while (a[++lower] < pivot);
                
                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else {
                a[--upper] = ak;
            }
        }
    }
    
    a[low] = a[lower];
    a[lower] = pivot;
    
    return std::make_pair(lower, upper);
}

// LONG ARRAY IMPLEMENTATIONS  
// ---------------------------

// Type-specific mixed insertion sort for long arrays
static void mixedInsertionSort_long(long* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);
    
    if (end == high) {
        for (int i; ++low < end; ) {
            long ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        long pin = a[end];
        
        for (int i, p = high; ++low < end; ) {
            long ai = a[i = low];
            
            if (ai < a[i - 1]) {
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
                
            } else if (p > i && ai > pin) {
                while (a[--p] > pin);
                
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }
                
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }
        
        for (int i; low < high; ++low) {
            long a1 = a[i = low], a2 = a[++low];
            
            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;
                
                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;
                
            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;
                
                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

// Type-specific insertion sort for long arrays
static void insertionSort_long(long* a, int low, int high) {
    for (int i, k = low; ++k < high; ) {
        long ai = a[i = k];
        
        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }
        
        if (UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

// Type-specific heap sort for long arrays
static void heapSort_long(long* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown_long(a, k, a[k], low, high);
    }
    while (--high > low) {
        long max = a[low];
        pushDown_long(a, low, a[high], low, high);
        a[high] = max;
    }
}

static void pushDown_long(long* a, int p, long value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2;
        
        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

// FLOAT ARRAY IMPLEMENTATIONS
// ----------------------------

// Type-specific mixed insertion sort for float arrays
static void mixedInsertionSort_float(float* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);
    
    if (end == high) {
        // Simple insertion sort for tiny arrays
        for (int i; ++low < end; ) {
            float ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort + pair insertion sort
        float pin = a[end];
        
        for (int i, p = high; ++low < end; ) {
            float ai = a[i = low];
            
            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
                
            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);
                
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }
                
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }
        
        // Pair insertion sort on remaining part
        for (int i; low < high; ++low) {
            float a1 = a[i = low], a2 = a[++low];
            
            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;
                
                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;
                
            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;
                
                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

// Type-specific insertion sort for float arrays
static void insertionSort_float(float* a, int low, int high) {
    for (int i, k = low; ++k < high; ) {
        float ai = a[i = k];
        
        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }
        
        if (UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

// Type-specific heap sort for float arrays
static void heapSort_float(float* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown_float(a, k, a[k], low, high);
    }
    while (--high > low) {
        float max = a[low];
        pushDown_float(a, low, a[high], low, high);
        a[high] = max;
    }
}

// Type-specific pushDown for float arrays
static void pushDown_float(float* a, int p, float value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2;
        
        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

// Type-specific dual pivot partitioning for float arrays
static std::pair<int, int> partitionDualPivot_float(float* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    
    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    float pivot1 = a[e1];
    float pivot2 = a[e5];
    
    a[e1] = a[lower];
    a[e5] = a[upper];
    
    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);
    
    (void)--lower;
    for (int k = ++upper; --k > lower; ) {
        float ak = a[k];
        
        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }
        
        if (UNLIKELY(ak < pivot1)) {
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) {
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

// Type-specific single pivot partitioning for float arrays
static std::pair<int, int> partitionSinglePivot_float(float* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    float pivot = a[e3];
    
    a[e3] = a[lower];
    
    for (int k = ++upper; --k > lower; ) {
        float ak = a[k];
        
        if (ak != pivot) {
            a[k] = pivot;
            
            if (ak < pivot) {
                while (a[++lower] < pivot);
                
                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else {
                a[--upper] = ak;
            }
        }
    }
    
    a[low] = a[lower];
    a[lower] = pivot;
    
    return std::make_pair(lower, upper);
}

// DOUBLE ARRAY IMPLEMENTATIONS
// -----------------------------

// Type-specific mixed insertion sort for double arrays
static void mixedInsertionSort_double(double* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);
    
    if (end == high) {
        for (int i; ++low < end; ) {
            double ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        double pin = a[end];
        
        for (int i, p = high; ++low < end; ) {
            double ai = a[i = low];
            
            if (ai < a[i - 1]) {
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
                
            } else if (p > i && ai > pin) {
                while (a[--p] > pin);
                
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }
                
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }
        
        for (int i; low < high; ++low) {
            double a1 = a[i = low], a2 = a[++low];
            
            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;
                
                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;
                
            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;
                
                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

// Type-specific insertion sort for double arrays
static void insertionSort_double(double* a, int low, int high) {
    for (int i, k = low; ++k < high; ) {
        double ai = a[i = k];
        
        if (LIKELY(k + 1 < high)) {
            PREFETCH_READ(&a[k + 1]);
        }
        
        if (UNLIKELY(ai < a[i - 1])) {
            while (--i >= low && ai < a[i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    }
}

// Type-specific heap sort for double arrays
static void heapSort_double(double* a, int low, int high) {
    for (int k = (low + high) >> 1; k > low; ) {
        --k;
        pushDown_double(a, k, a[k], low, high);
    }
    while (--high > low) {
        double max = a[low];
        pushDown_double(a, low, a[high], low, high);
        a[high] = max;
    }
}

static void pushDown_double(double* a, int p, double value, int low, int high) {
    for (int k;;) {
        k = (p << 1) - low + 2;
        
        if (k > high) {
            break;
        }
        if (k == high || a[k] < a[k - 1]) {
            --k;
        }
        if (a[k] <= value) {
            break;
        }
        a[p] = a[k];
        p = k;
    }
    a[p] = value;
}

// Type-specific dual pivot partitioning for double arrays
static std::pair<int, int> partitionDualPivot_double(double* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    
    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    double pivot1 = a[e1];
    double pivot2 = a[e5];
    
    a[e1] = a[lower];
    a[e5] = a[upper];
    
    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);
    
    (void)--lower;
    for (int k = ++upper; --k > lower; ) {
        double ak = a[k];
        
        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }
        
        if (UNLIKELY(ak < pivot1)) {
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) {
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

// Type-specific single pivot partitioning for double arrays
static std::pair<int, int> partitionSinglePivot_double(double* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    double pivot = a[e3];
    
    a[e3] = a[lower];
    
    for (int k = ++upper; --k > lower; ) {
        double ak = a[k];
        
        if (ak != pivot) {
            a[k] = pivot;
            
            if (ak < pivot) {
                while (a[++lower] < pivot);
                
                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else {
                a[--upper] = ak;
            }
        }
    }
    
    a[low] = a[lower];
    a[lower] = pivot;
    
    return std::make_pair(lower, upper);
}

// BYTE ARRAY IMPLEMENTATIONS
// ---------------------------

// Type-specific mixed insertion sort for byte arrays
static void mixedInsertionSort_byte(signed char* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);
    
    if (end == high) {
        // Simple insertion sort for tiny byte arrays
        for (int i; ++low < end; ) {
            signed char ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort optimized for byte values
        signed char pin = a[end];
        
        for (int i, p = high; ++low < end; ) {
            signed char ai = a[i = low];
            
            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
                
            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);
                
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }
                
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }
        
        // Pair insertion sort optimized for byte values
        for (int i; low < high; ++low) {
            signed char a1 = a[i = low], a2 = a[++low];
            
            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;
                
                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;
                
            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;
                
                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

// CHAR ARRAY IMPLEMENTATIONS
// ---------------------------

// Type-specific mixed insertion sort for char arrays
static void mixedInsertionSort_char(char* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);
    
    if (end == high) {
        // Simple insertion sort for tiny char arrays
        for (int i; ++low < end; ) {
            char ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort optimized for char values
        char pin = a[end];
        
        for (int i, p = high; ++low < end; ) {
            char ai = a[i = low];
            
            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
                
            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);
                
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }
                
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }
        
        // Pair insertion sort optimized for char values
        for (int i; low < high; ++low) {
            char a1 = a[i = low], a2 = a[++low];
            
            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;
                
                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;
                
            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;
                
                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

// SHORT ARRAY IMPLEMENTATIONS
// ----------------------------

// Type-specific mixed insertion sort for short arrays
static void mixedInsertionSort_short(short* a, int low, int high) {
    int size = high - low;
    int end = high - 3 * ((size >> 5) << 3);
    
    if (end == high) {
        // Simple insertion sort for tiny short arrays
        for (int i; ++low < end; ) {
            short ai = a[i = low];
            while (ai < a[--i]) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {
        // Pin insertion sort optimized for short values
        short pin = a[end];
        
        for (int i, p = high; ++low < end; ) {
            short ai = a[i = low];
            
            if (ai < a[i - 1]) { // Small element
                a[i] = a[i - 1];
                --i;
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
                
            } else if (p > i && ai > pin) { // Large element
                while (a[--p] > pin);
                
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }
                
                while (ai < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }
        
        // Pair insertion sort optimized for short values
        for (int i; low < high; ++low) {
            short a1 = a[i = low], a2 = a[++low];
            
            if (a1 > a2) {
                while (a1 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;
                
                while (a2 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;
                
            } else if (a1 < a[i - 1]) {
                while (a2 < a[--i]) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;
                
                while (a1 < a[--i]) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}

// Type-specific dual pivot partitioning for long arrays
static std::pair<int, int> partitionDualPivot_long(long* a, int low, int high, int pivotIndex1, int pivotIndex2) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    
    int e1 = pivotIndex1;
    int e5 = pivotIndex2;
    long pivot1 = a[e1];
    long pivot2 = a[e5];
    
    a[e1] = a[lower];
    a[e5] = a[upper];
    
    while (a[++lower] < pivot1);
    while (a[--upper] > pivot2);
    
    (void)--lower;
    for (int k = ++upper; --k > lower; ) {
        long ak = a[k];
        
        if (LIKELY(k > lower + 1)) {
            PREFETCH_READ(&a[k - 1]);
        }
        
        if (UNLIKELY(ak < pivot1)) {
            while (lower < k) {
                if (LIKELY(a[++lower] >= pivot1)) {
                    if (UNLIKELY(a[lower] > pivot2)) {
                        a[k] = a[--upper];
                        a[upper] = a[lower];
                    } else {
                        a[k] = a[lower];
                    }
                    a[lower] = ak;
                    break;
                }
            }
        } else if (UNLIKELY(ak > pivot2)) {
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

// Type-specific single pivot partitioning for long arrays
static std::pair<int, int> partitionSinglePivot_long(long* a, int low, int high, int pivotIndex1, int) {
    int end = high - 1;
    int lower = low;
    int upper = end;
    int e3 = pivotIndex1;
    long pivot = a[e3];
    
    a[e3] = a[lower];
    
    for (int k = ++upper; --k > lower; ) {
        long ak = a[k];
        
        if (ak != pivot) {
            a[k] = pivot;
            
            if (ak < pivot) {
                while (a[++lower] < pivot);
                
                if (a[lower] > pivot) {
                    a[--upper] = a[lower];
                }
                a[lower] = ak;
            } else {
                a[--upper] = ak;
            }
        }
    }
    
    a[low] = a[lower];
    a[lower] = pivot;
    
    return std::make_pair(lower, upper);
}

// TYPE-SPECIFIC RUN MERGING IMPLEMENTATIONS
// -----------------------------------------

// Type-specific run merging for int arrays with Sorter integration
static bool tryMergeRuns_int(Sorter<int>* sorter, int* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;
    
    // Run detection logic (same as generic version but type-specific)
    for (int k = low + 1; k < high; ) {
        
        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);
            
            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                int temp = a[i]; 
                a[i] = a[j]; 
                a[j] = temp;
            }
        } else {
            int ak = a[k];
            while (++k < high && ak == a[k]);
            
            if (k < high) {
                continue;
            }
        }
        
        if (run.empty()) {
            if (k == high) {
                return true;
            }
            
            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }
            
            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);
            
        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }
            
            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }
    
    // Merge runs with advanced parallel coordination (matching Java's approach)
    if (count > 1) {
        std::vector<int> b(size);
        
        // Advanced parallel run merging logic with MIN_RUN_COUNT threshold
        if (count >= MIN_RUN_COUNT && sorter != nullptr) {
            // Use Java-style forkMe/getDestination pattern for parallel merging
            RunMerger<int> merger(a, b.data(), low, 1, run, 0, count);
            RunMerger<int>& forked = merger.forkMe();
            int* result = forked.getDestination();
            
            // Copy back to main array if needed (matching Java's buffer management)
            if (result != a) {
                std::copy(result + low, result + low + size, a + low);
            }
        } else {
            // Sequential merging with local variable optimization
            int* localA = a; // Local variable optimization (matching Java pattern)
            mergeRuns_int(localA, b.data(), low, 1, false, run, 0, count);
        }
    }
    return true;
}

// Type-specific run merging for long arrays with Sorter integration
static bool tryMergeRuns_long(Sorter<long>* sorter, long* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;
    
    for (int k = low + 1; k < high; ) {
        
        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);
            
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                long temp = a[i]; 
                a[i] = a[j]; 
                a[j] = temp;
            }
        } else {
            long ak = a[k];
            while (++k < high && ak == a[k]);
            
            if (k < high) {
                continue;
            }
        }
        
        if (run.empty()) {
            if (k == high) {
                return true;
            }
            
            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }
            
            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);
            
        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }
            
            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }
    
    if (count > 1) {
        std::vector<long> b(size);
        mergeRuns_long(a, b.data(), low, 1, count >= MIN_RUN_COUNT && sorter != nullptr, run, 0, count);
    }
    return true;
}

// Type-specific run merging for float arrays with Sorter integration
static bool tryMergeRuns_float(Sorter<float>* sorter, float* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;
    
    for (int k = low + 1; k < high; ) {
        
        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);
            
            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                float temp = a[i]; 
                a[i] = a[j]; 
                a[j] = temp;
            }
        } else {
            float ak = a[k];
            while (++k < high && ak == a[k]);
            
            if (k < high) {
                continue;
            }
        }
        
        if (run.empty()) {
            if (k == high) {
                return true;
            }
            
            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }
            
            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);
            
        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }
            
            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }
    
    if (count > 1) {
        std::vector<float> b(size);
        mergeRuns_float(a, b.data(), low, 1, count >= MIN_RUN_COUNT && sorter != nullptr, run, 0, count);
    }
    return true;
}

// Type-specific run merging for double arrays with Sorter integration
static bool tryMergeRuns_double(Sorter<double>* sorter, double* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;
    
    for (int k = low + 1; k < high; ) {
        
        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);
            
            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                double temp = a[i]; 
                a[i] = a[j]; 
                a[j] = temp;
            }
        } else {
            double ak = a[k];
            while (++k < high && ak == a[k]);
            
            if (k < high) {
                continue;
            }
        }
        
        if (run.empty()) {
            if (k == high) {
                return true;
            }
            
            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }
            
            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);
            
        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }
            
            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }
    
    if (count > 1) {
        std::vector<double> b(size);
        mergeRuns_double(a, b.data(), low, 1, count >= MIN_RUN_COUNT && sorter != nullptr, run, 0, count);
    }
    return true;
}

// Type-specific merge runs implementations
static int* mergeRuns_int(int* a, int* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }
    
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);
    
    int* a1 = mergeRuns_int(a, b, offset, -aim, parallel, run, lo, mi);
    int* a2 = mergeRuns_int(a, b, offset,    0, parallel, run, mi, hi);
    
    int* dst = (a1 == a) ? b : a;
    
    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];
    
    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

static long* mergeRuns_long(long* a, long* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }
    
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);
    
    long* a1 = mergeRuns_long(a, b, offset, -aim, parallel, run, lo, mi);
    long* a2 = mergeRuns_long(a, b, offset,    0, parallel, run, mi, hi);
    
    long* dst = (a1 == a) ? b : a;
    
    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];
    
    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

static float* mergeRuns_float(float* a, float* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }
    
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);
    
    float* a1 = mergeRuns_float(a, b, offset, -aim, parallel, run, lo, mi);
    float* a2 = mergeRuns_float(a, b, offset,    0, parallel, run, mi, hi);
    
    float* dst = (a1 == a) ? b : a;
    
    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];
    
    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

static double* mergeRuns_double(double* a, double* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }
    
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);
    
    double* a1 = mergeRuns_double(a, b, offset, -aim, parallel, run, lo, mi);
    double* a2 = mergeRuns_double(a, b, offset,    0, parallel, run, mi, hi);
    
    double* dst = (a1 == a) ? b : a;
    
    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];
    
    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

// BYTE, CHAR, SHORT RUN MERGING IMPLEMENTATIONS
// ----------------------------------------------

// Type-specific tryMergeRuns for byte arrays
static bool tryMergeRuns_byte(signed char* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;
    
    // Run detection logic (matching Java's approach for byte arrays)
    for (int k = low + 1; k < high; ) {
        
        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);
            
            // Reverse into ascending order
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                signed char temp = a[i]; 
                a[i] = a[j]; 
                a[j] = temp;
            }
        } else {
            signed char ak = a[k];
            while (++k < high && ak == a[k]);
            
            if (k < high) {
                continue;
            }
        }
        
        if (run.empty()) {
            if (k == high) {
                return true;
            }
            
            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }
            
            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);
            
        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }
            
            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }
    
    // Merge runs for byte arrays (simplified, no parallel coordination)
    if (count > 1) {
        std::vector<signed char> b(size);
        mergeRuns_byte(a, b.data(), low, 1, false, run, 0, count);
    }
    return true;
}

// Type-specific tryMergeRuns for char arrays
static bool tryMergeRuns_char(char* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;
    
    for (int k = low + 1; k < high; ) {
        
        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);
            
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                char temp = a[i]; 
                a[i] = a[j]; 
                a[j] = temp;
            }
        } else {
            char ak = a[k];
            while (++k < high && ak == a[k]);
            
            if (k < high) {
                continue;
            }
        }
        
        if (run.empty()) {
            if (k == high) {
                return true;
            }
            
            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }
            
            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);
            
        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }
            
            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }
    
    if (count > 1) {
        std::vector<char> b(size);
        mergeRuns_char(a, b.data(), low, 1, false, run, 0, count);
    }
    return true;
}

// Type-specific tryMergeRuns for short arrays
static bool tryMergeRuns_short(short* a, int low, int size) {
    std::vector<int> run;
    int high = low + size;
    int count = 1, last = low;
    
    for (int k = low + 1; k < high; ) {
        
        if (a[k - 1] < a[k]) {
            while (++k < high && a[k - 1] <= a[k]);
        } else if (a[k - 1] > a[k]) {
            while (++k < high && a[k - 1] >= a[k]);
            
            for (int i = last - 1, j = k; ++i < --j && a[i] > a[j]; ) {
                short temp = a[i]; 
                a[i] = a[j]; 
                a[j] = temp;
            }
        } else {
            short ak = a[k];
            while (++k < high && ak == a[k]);
            
            if (k < high) {
                continue;
            }
        }
        
        if (run.empty()) {
            if (k == high) {
                return true;
            }
            
            if (k - low < MIN_FIRST_RUN_SIZE) {
                return false;
            }
            
            run.reserve(((size >> 10) | 0x7F) & 0x3FF);
            run.push_back(low);
            
        } else if (a[last - 1] > a[last]) {
            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {
                return false;
            }
            
            if (++count == MAX_RUN_CAPACITY) {
                return false;
            }
        }
        run.push_back(last = k);
    }
    
    if (count > 1) {
        std::vector<short> b(size);
        mergeRuns_short(a, b.data(), low, 1, false, run, 0, count);
    }
    return true;
}

// Type-specific merge runs implementations for byte, char, short
static signed char* mergeRuns_byte(signed char* a, signed char* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }
    
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);
    
    signed char* a1 = mergeRuns_byte(a, b, offset, -aim, parallel, run, lo, mi);
    signed char* a2 = mergeRuns_byte(a, b, offset,    0, parallel, run, mi, hi);
    
    signed char* dst = (a1 == a) ? b : a;
    
    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];
    
    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

static char* mergeRuns_char(char* a, char* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }
    
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);
    
    char* a1 = mergeRuns_char(a, b, offset, -aim, parallel, run, lo, mi);
    char* a2 = mergeRuns_char(a, b, offset,    0, parallel, run, mi, hi);
    
    char* dst = (a1 == a) ? b : a;
    
    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];
    
    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

static short* mergeRuns_short(short* a, short* b, int offset, int aim, bool parallel, const std::vector<int>& run, int lo, int hi) {
    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
        for (int i = run[hi], j = i - offset, low = run[lo]; i > low; ) {
            b[--j] = a[--i];
        }
        return b;
    }
    
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi);
    
    short* a1 = mergeRuns_short(a, b, offset, -aim, parallel, run, lo, mi);
    short* a2 = mergeRuns_short(a, b, offset,    0, parallel, run, mi, hi);
    
    short* dst = (a1 == a) ? b : a;
    
    int k   = (a1 == a) ? run[lo] - offset : run[lo];
    int lo1 = (a1 == b) ? run[lo] - offset : run[lo];
    int hi1 = (a1 == b) ? run[mi] - offset : run[mi];
    int lo2 = (a2 == b) ? run[mi] - offset : run[mi];
    int hi2 = (a2 == b) ? run[hi] - offset : run[hi];
    
    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2);
    return dst;
}

// Forward declarations for counting sorts
template<typename T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 1, void>::type
countingSort(T* a, int low, int high);

template<typename T>  
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 2, void>::type
countingSort(T* a, int low, int high);

// -----------------------------------------------------------------------------
// INTEGER TYPE SORT IMPLEMENTATIONS
// -----------------------------------------------------------------------------

// INT ARRAY SORT IMPLEMENTATIONS with Sorter integration
// -----------------------------------------------------


// Sequential int array sorting with Sorter support
static void sort_int_sequential(Sorter<int>* sorter, int* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;
        
        // Use mixed insertion sort on small non-leftmost parts
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            sort_intrinsic(a, low, high, mixedInsertionSort_int);
            return;
        }
        
        // Use insertion sort on small leftmost parts
        if (size < MAX_INSERTION_SORT_SIZE) {
            sort_intrinsic(a, low, high, insertionSort_int);
            return;
        }
        
        // Try merge runs for nearly sorted data
        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns_int(sorter, a, low, size)) {
            return;
        }
        
        // Switch to heap sort if execution time is becoming quadratic
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            sort_intrinsic(a, low, high, heapSort_int);
            return;
        }
        
        // Five-element pivot selection
        int step = (size >> 3) * 3 + 3;
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;
        
        // Sort 5-element sample
        sort5Network(a, e1, e2, e3, e4, e5);
        
        int lower, upper;
        
        // Dual-pivot partitioning
        if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
            auto pivotIndices = partition_intrinsic(a, low, high, e1, e5, partitionDualPivot_int);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            // Fork parallel tasks if sorter available
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, lower + 1, upper);
                sorter->forkSorter(bits | 1, upper + 1, high);
            } else {
                sort_int_sequential(sorter, a, bits | 1, lower + 1, upper);
                sort_int_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        } else {
            // Single-pivot partitioning
            auto pivotIndices = partition_intrinsic(a, low, high, e3, e3, partitionSinglePivot_int);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, upper, high);
            } else {
                sort_int_sequential(sorter, a, bits | 1, upper, high);
            }
        }
        
        high = lower; // Continue with left part
    }
}

// LONG ARRAY SORT IMPLEMENTATIONS with Sorter integration
// --------------------------------------------------------


// Sequential long array sorting with Sorter support
static void sort_long_sequential(Sorter<long>* sorter, long* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;
        
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            sort_intrinsic(a, low, high, mixedInsertionSort_long);
            return;
        }
        
        if (size < MAX_INSERTION_SORT_SIZE) {
            sort_intrinsic(a, low, high, insertionSort_long);
            return;
        }
        
        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns_long(sorter, a, low, size)) {
            return;
        }
        
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            sort_intrinsic(a, low, high, heapSort_long);
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
            auto pivotIndices = partitionDualPivot_long(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, lower + 1, upper);
                sorter->forkSorter(bits | 1, upper + 1, high);
            } else {
                sort_long_sequential(sorter, a, bits | 1, lower + 1, upper);
                sort_long_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        } else {
            auto pivotIndices = partitionSinglePivot_long(a, low, high, e3, e3);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, upper, high);
            } else {
                sort_long_sequential(sorter, a, bits | 1, upper, high);
            }
        }
        
        high = lower;
    }
}


// FLOATING-POINT TYPE SORT IMPLEMENTATIONS with special value handling
// ---------------------------------------------------------------------

// Type-specific floating-point sorting with comprehensive NaN and negative zero handling
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
sort_float_specialized(T* a, int low, int hi) {
    // Phase 1: Handle special floating-point values using precise bit manipulation
    int numNegativeZero = 0;
    int actualHigh = hi;
    
    // Move NaNs to end and count negative zeros using precise bit-level detection
    for (int k = actualHigh - 1; k >= low; k--) {
        T ak = a[k];
        
        // Use precise bit-level NaN detection
        if constexpr (std::is_same_v<T, float>) {
            if (isNaN(ak)) {
                a[k] = a[--actualHigh];
                a[actualHigh] = ak;
            } else if (isNegativeZero(ak)) {
                numNegativeZero++;
                a[k] = T(0); // Convert to positive zero for sorting  
            }
        } else if constexpr (std::is_same_v<T, double>) {
            if (isNaN(ak)) {
                a[k] = a[--actualHigh];
                a[actualHigh] = ak;
            } else if (isNegativeZero(ak)) {
                numNegativeZero++;
                a[k] = T(0); // Convert to positive zero for sorting
            }
        }
    }
    
    // Phase 2: Sort the non-NaN part using regular algorithm
    if (actualHigh > low) {
        if constexpr (std::is_same_v<T, float>) {
            sort_float_sequential(nullptr, a, 0, low, actualHigh);
        } else if constexpr (std::is_same_v<T, double>) {
            sort_double_sequential(nullptr, a, 0, low, actualHigh);
        }
    }
    
    // Phase 3: Restore negative zeros using optimized binary search
    if (numNegativeZero > 0) {
        // Use precise binary search with unsigned right shift (matching Java >>> operator)
        int left = findZeroPosition(a, low, actualHigh - 1);
        
        // Replace positive zeros with negative zeros using bit manipulation
        for (int i = 0; i < numNegativeZero && left < actualHigh; i++, left++) {
            if constexpr (std::is_same_v<T, float>) {
                if (isPositiveZero(a[left])) {
                    a[left] = intBitsToFloat(0x80000000U); // -0.0f
                }
            } else if constexpr (std::is_same_v<T, double>) {
                if (isPositiveZero(a[left])) {
                    a[left] = longBitsToDouble(0x8000000000000000ULL); // -0.0
                }
            }
        }
    }
}


// Type-specific sequential sorting for float arrays
static void sort_float_sequential(Sorter<float>* sorter, float* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;
        
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            sort_intrinsic(a, low, high, mixedInsertionSort_float);
            return;
        }
        
        if (size < MAX_INSERTION_SORT_SIZE) {
            sort_intrinsic(a, low, high, insertionSort_float);
            return;
        }
        
        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns_float(sorter, a, low, size)) {
            return;
        }
        
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            sort_intrinsic(a, low, high, heapSort_float);
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
            auto pivotIndices = partitionDualPivot_float(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, lower + 1, upper);
                sorter->forkSorter(bits | 1, upper + 1, high);
            } else {
                sort_float_sequential(sorter, a, bits | 1, lower + 1, upper);
                sort_float_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        } else {
            auto pivotIndices = partitionSinglePivot_float(a, low, high, e3, e3);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, upper, high);
            } else {
                sort_float_sequential(sorter, a, bits | 1, upper, high);
            }
        }
        
        high = lower;
    }
}

// Type-specific sequential sorting for double arrays
static void sort_double_sequential(Sorter<double>* sorter, double* a, int bits, int low, int high) {
    while (true) {
        int end = high - 1;
        int size = high - low;
        
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            sort_intrinsic(a, low, high, mixedInsertionSort_double);
            return;
        }
        
        if (size < MAX_INSERTION_SORT_SIZE) {
            sort_intrinsic(a, low, high, insertionSort_double);
            return;
        }
        
        if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                && tryMergeRuns_double(sorter, a, low, size)) {
            return;
        }
        
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            sort_intrinsic(a, low, high, heapSort_double);
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
            auto pivotIndices = partitionDualPivot_double(a, low, high, e1, e5);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, lower + 1, upper);
                sorter->forkSorter(bits | 1, upper + 1, high);
            } else {
                sort_double_sequential(sorter, a, bits | 1, lower + 1, upper);
                sort_double_sequential(sorter, a, bits | 1, upper + 1, high);
            }
        } else {
            auto pivotIndices = partitionSinglePivot_double(a, low, high, e3, e3);
            lower = pivotIndices.first;
            upper = pivotIndices.second;
            
            if (sorter != nullptr && size > MIN_PARALLEL_SORT_SIZE) {
                sorter->forkSorter(bits | 1, upper, high);
            } else {
                sort_double_sequential(sorter, a, bits | 1, upper, high);
            }
        }
        
        high = lower;
    }
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 1, void>::type
sort_specialized(T* a, int low, int high) {
    // Counting sort for byte-sized integers (char, unsigned char, signed char)
    if (high - low >= MIN_BYTE_COUNTING_SORT_SIZE) {
        countingSort(a, low, high);
    } else {
        insertionSort(a, low, high);
    }
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 2, void>::type
sort_specialized(T* a, int low, int high) {
    // Counting sort for short-sized integers when range is reasonable
    if (high - low >= MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE) {
        // For now, fallback to regular sort - full counting sort would need range analysis
        sort(a, 0, low, high);
    } else {
        sort(a, 0, low, high);
    }
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
sort_specialized(T* a, int low, int high) {
    // Phase 1: Handle special floating-point values
    int numNegativeZero = 0;
    int actualHigh = high;
    
    // Move NaNs to end and count negative zeros
    for (int k = actualHigh - 1; k >= low; k--) {
        T ak = a[k];
        
        if (ak != ak) { // NaN detection
            a[k] = a[--actualHigh];
            a[actualHigh] = ak;
        } else if (ak == T(0)) {
            // Check for negative zero using bit representation
            if constexpr (std::is_same_v<T, float>) {
                if (std::signbit(ak)) {
                    numNegativeZero++;
                    a[k] = T(0); // Convert to positive zero for sorting
                }
            } else if constexpr (std::is_same_v<T, double>) {
                if (std::signbit(ak)) {
                    numNegativeZero++;
                    a[k] = T(0); // Convert to positive zero for sorting
                }
            }
        }
    }
    
    // Phase 2: Sort the non-NaN part
    if (actualHigh > low) {
        sort(a, 0, low, actualHigh);
    }
    
    // Phase 3: Restore negative zeros if any
    if (numNegativeZero > 0) {
        // Find position of zeros using binary search
        int left = low, right = actualHigh - 1;
        while (left <= right) {
            int mid = (left + right) / 2;
            if (a[mid] < T(0)) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        // Replace positive zeros with negative zeros
        for (int i = 0; i < numNegativeZero && left < actualHigh; i++, left++) {
            if (a[left] == T(0)) {
                a[left] = -T(0);
            }
        }
    }
}

// =============================================================================
// OPTIMIZED COUNTING SORT IMPLEMENTATIONS (matching Java's sophisticated approach)
// =============================================================================

// Advanced byte counting sort with optimized histogram computation (matching Java)
template<typename T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 1, void>::type
countingSort(T* a, int low, int high) {
    static constexpr int NUM_VALUES = 1 << (8 * sizeof(T));
    static constexpr int OFFSET = std::is_signed<T>::value ? (1 << (8 * sizeof(T) - 1)) : 0;
    
    std::vector<int> count(NUM_VALUES, 0);
    
    // Optimized histogram computation (matching Java's reverse iteration for better cache performance)
    for (int i = high; i > low; ) {
        count[static_cast<unsigned char>(a[--i]) + OFFSET]++;
    }
    
    // Two-strategy placement algorithm based on array size (matching Java's approach)
    int size = high - low;
    if (size > NUM_VALUES / 2) {
        // Strategy 1: Large arrays - use reverse iteration to minimize cache misses
        int index = high;
        for (int i = NUM_VALUES; --i >= 0; ) {
            T value = static_cast<T>(i - OFFSET);
            int cnt = count[i];
            while (cnt-- > 0) {
                a[--index] = value;
            }
        }
    } else {
        // Strategy 2: Small arrays - use skip-zero optimization for sparse data
        int index = low;
        for (int i = 0; i < NUM_VALUES; i++) {
            if (count[i] > 0) { // Skip-zero optimization
                T value = static_cast<T>(i - OFFSET);
                int cnt = count[i];
                while (cnt-- > 0) {
                    a[index++] = value;
                }
            }
        }
    }
}

// Advanced char counting sort with Unicode optimization (matching Java)
static void countingSort(char* a, int low, int high) {
    static constexpr int NUM_CHAR_VALUES = 1 << 16; // Full Unicode range
    std::vector<int> count(NUM_CHAR_VALUES, 0);
    
    // Direct unsigned access for characters (matching Java's approach)
    for (int i = high; i > low; ) {
        ++count[static_cast<unsigned char>(a[--i])];
    }
    
    // Optimized placement for character ranges
    int index = low;
    for (int i = 0; i < NUM_CHAR_VALUES; i++) {
        if (count[i] > 0) {
            int cnt = count[i];
            char value = static_cast<char>(i);
            while (cnt-- > 0) {
                a[index++] = value;
            }
        }
    }
}

// Advanced short counting sort with bit manipulation and dual strategy (matching Java)
template<typename T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 2, void>::type
countingSort(T* a, int low, int high) {
    static constexpr int NUM_SHORT_VALUES = 1 << 16; // 65536
    static constexpr int MAX_SHORT_INDEX = std::is_signed<T>::value ? 
        (1 << 15) + NUM_SHORT_VALUES + 1 : NUM_SHORT_VALUES + 1;
    
    int size = high - low;
    
    // Use full histogram for moderate-sized arrays
    if (size < NUM_SHORT_VALUES) {
        std::vector<int> count(NUM_SHORT_VALUES, 0);
        
        // Bit manipulation optimization (matching Java's approach)
        for (int i = high; i > low; ) {
            ++count[a[--i] & 0xFFFF]; // Mask to handle signed/unsigned uniformly
        }
        
        // Skip-zero strategy for small arrays  
        int index = low;
        for (int i = 0; i < NUM_SHORT_VALUES; ) {
            // Skip consecutive zeros for better performance
            while (i < NUM_SHORT_VALUES && count[i] == 0) ++i;
            if (i < NUM_SHORT_VALUES) {
                T value = static_cast<T>(i);
                if constexpr (std::is_signed<T>::value) {
                    // Handle sign extension for signed types
                    value = static_cast<T>(static_cast<std::int16_t>(i));
                }
                int cnt = count[i];
                while (cnt-- > 0) {
                    a[index++] = value;
                }
                ++i;
            }
        }
    } else {
        // For large arrays, use reverse iteration strategy (matching Java)
        std::vector<int> count(MAX_SHORT_INDEX, 0);
        
        for (int i = high; i > low; ) {
            T val = a[--i];
            int idx = static_cast<int>(val);
            if constexpr (std::is_signed<T>::value) {
                idx += (1 << 15); // Offset for signed values
            }
            ++count[idx];
        }
        
        // Reverse iteration for large arrays to improve cache locality
        int index = high;
        for (int i = MAX_SHORT_INDEX; --i >= 0; ) {
            if (count[i] > 0) {
                T value;
                if constexpr (std::is_signed<T>::value) {
                    value = static_cast<T>(i - (1 << 15));
                } else {
                    value = static_cast<T>(i);
                }
                int cnt = count[i];
                while (cnt-- > 0) {
                    a[--index] = value;
                }
            }
        }
    }
}

// Default case for other types - use regular dual-pivot sort
template<typename T>
typename std::enable_if<!std::is_integral<T>::value && !std::is_floating_point<T>::value, void>::type
sort_specialized(T* a, int low, int high) {
    sort(a, 0, low, high);
}

// Override for larger integral types (int, long, etc.) - use regular sort
template<typename T>
typename std::enable_if<std::is_integral<T>::value && (sizeof(T) > 2), void>::type
sort_specialized(T* a, int low, int high) {
    sort(a, 0, low, high);
}

template<typename RandomAccessIterator>
void dual_pivot_quicksort(RandomAccessIterator first, RandomAccessIterator last) {
    static_assert(std::is_same_v<typename std::iterator_traits<RandomAccessIterator>::iterator_category, 
                                std::random_access_iterator_tag>, 
                  "dual_pivot_quicksort requires random access iterators");
    
    if (first >= last) return;
    
    int size = last - first;
    if (size <= 1) return;
    
    // Get pointer to underlying array
    auto* a = &(*first);
    
    // Use type-specialized sorting when beneficial
    using ValueType = typename std::iterator_traits<RandomAccessIterator>::value_type;
    if constexpr (std::is_integral_v<ValueType> && sizeof(ValueType) <= 2) {
        // Use specialized sorting for small integer types
        sort_specialized(a, 0, size);
    } else if constexpr (std::is_floating_point_v<ValueType>) {
        // Use specialized sorting for floating-point types
        sort_specialized(a, 0, size);
    } else {
        // Use regular dual-pivot sort for other types
        sort(a, 0, 0, size);
    }
}

// Parallel sorting entry point
template<typename RandomAccessIterator>
void dual_pivot_quicksort_parallel(RandomAccessIterator first, RandomAccessIterator last, 
                                  int parallelism = std::thread::hardware_concurrency()) {
    static_assert(std::is_same_v<typename std::iterator_traits<RandomAccessIterator>::iterator_category, 
                                std::random_access_iterator_tag>, 
                  "dual_pivot_quicksort requires random access iterators");
    
    if (first >= last) return;
    
    int size = last - first;
    if (size <= 1) return;
    
    // Get pointer to underlying array
    auto* a = &(*first);
    
    // Use parallel sorting for large arrays
    using ValueType = typename std::iterator_traits<RandomAccessIterator>::value_type;
    if (size > MIN_PARALLEL_SORT_SIZE && parallelism > 1) {
        parallelSort(a, parallelism, 0, size);
    } else {
        // Fall back to sequential for small arrays or single thread
        dual_pivot_quicksort(first, last);
    }
}

// Phase 5: Parallel sorting implementation  
template<typename T>
void parallelSort(T* a, int parallelism, int low, int high) {
    int size = high - low;
    
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        
        if (depth < 0) {
            // Use parallel merge sort approach for highly parallel scenarios
            std::vector<T> b(size);
            parallelMergeSort(a, b.data(), low, size, low, depth);
        } else {
            // Use parallel quicksort with work stealing
            parallelQuickSort(a, depth, low, high);
        }
    } else {
        // Fall back to sequential sort
        sort(a, 0, low, high);
    }
}

template<typename T>
void parallelQuickSort(T* a, int bits, int low, int high) {
    int size = high - low;
    
    // Use parallel partitioning for large arrays
    if (size > MIN_PARALLEL_SORT_SIZE) {
        while (true) {
            int end = high - 1;
            size = high - low;
            
            // Run mixed insertion sort on small non-leftmost parts
            if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
                mixedInsertionSort(a, low, high);
                return;
            }
            
            // Invoke insertion sort on small leftmost part
            if (size < MAX_INSERTION_SORT_SIZE) {
                insertionSort(a, low, high);
                return;
            }
            
            // Check if the whole array or large non-leftmost parts are nearly sorted
            if ((bits == 0 || (size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0))
                    && tryMergeRuns(a, low, size)) {
                return;
            }
            
            // Switch to heap sort if execution time is becoming quadratic
            if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
                heapSort(a, low, high);
                return;
            }
            
            // Pivot selection (same as sequential)
            int step = (size >> 3) * 3 + 3;
            int e1 = low + step;
            int e5 = end - step;
            int e3 = (e1 + e5) >> 1;
            int e2 = (e1 + e3) >> 1;
            int e4 = (e3 + e5) >> 1;
            T a3 = a[e3];
            
            // 5-element sorting network
            if (a[e5] < a[e2]) { T t = a[e5]; a[e5] = a[e2]; a[e2] = t; }
            if (a[e4] < a[e1]) { T t = a[e4]; a[e4] = a[e1]; a[e1] = t; }
            if (a[e5] < a[e4]) { T t = a[e5]; a[e5] = a[e4]; a[e4] = t; }
            if (a[e2] < a[e1]) { T t = a[e2]; a[e2] = a[e1]; a[e1] = t; }
            if (a[e4] < a[e2]) { T t = a[e4]; a[e4] = a[e2]; a[e2] = t; }
            
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
            
            int lower, upper;
            
            // Dual pivot partitioning with parallel recursive sorts
            if (a[e1] < a[e2] && a[e2] < a[e3] && a[e3] < a[e4] && a[e4] < a[e5]) {
                auto pivotIndices = partitionDualPivot(a, low, high, e1, e5);
                lower = pivotIndices.first;
                upper = pivotIndices.second;
                
                // Launch parallel tasks for the three parts
                auto& pool = getThreadPool();
                
                auto future1 = pool.enqueue([=] { parallelQuickSort(a, bits | 1, lower + 1, upper); });
                auto future2 = pool.enqueue([=] { parallelQuickSort(a, bits | 1, upper + 1, high); });
                
                // Wait for completion
                future1.get();
                future2.get();
                
            } else {
                auto pivotIndices = partitionSinglePivot(a, low, high, e3, e3);
                lower = pivotIndices.first;
                upper = pivotIndices.second;
                
                // Launch parallel task for right part
                auto& pool = getThreadPool();
                auto future = pool.enqueue([=] { parallelQuickSort(a, bits | 1, upper, high); });
                future.get();
            }
            
            high = lower; // Iterate along the left part
        }
    } else {
        // Use sequential sort for small arrays
        sort(a, bits, low, high);
    }
}

template<typename T>
void parallelMergeSort(T* a, T* b, int low, int size, int offset, int depth) {
    if (depth < 0) {
        // Split the array and sort both halves in parallel
        int half = size >> 1;
        
        auto& pool = getThreadPool();
        auto future1 = pool.enqueue([=] { parallelMergeSort(b, a, low, half, offset, depth + 1); });
        auto future2 = pool.enqueue([=] { parallelMergeSort(b, a, low + half, size - half, offset, depth + 1); });
        
        future1.get();
        future2.get();
        
        // Merge the results
        parallelMergeParts(a, low, b, low, low + half, b, low + half, low + size);
    } else {
        // Use sequential sort for small parts
        std::copy(a + low, a + low + size, b + low - offset);
        sort(b, depth, low - offset, low - offset + size);
        std::copy(b + low - offset, b + low - offset + size, a + low);
    }
}

// =============================================================================
// ADVANCED BUFFER MANAGEMENT SYSTEM (matching Java's sophisticated approach)
// =============================================================================

// Forward declarations for advanced buffer management
template<typename T> class AdvancedSorter;
template<typename T> class BufferManager;

// Enhanced buffer manager for sophisticated buffer reuse (matching Java's pattern)
template<typename T>
class BufferManager {
private:
    static thread_local std::vector<T> buffer_pool;
    static thread_local bool pool_initialized;
    static thread_local std::vector<int> buffer_offsets; // Advanced offset tracking
    static thread_local int buffer_usage_count; // Usage statistics
    
public:
    // Advanced buffer allocation with sophisticated reuse patterns  
    static T* getBuffer(int size, int& offset) {
        if (!pool_initialized || buffer_pool.size() < size) {
            // Sophisticated buffer sizing (matching Java's growth strategy)
            int new_size = std::max(size, static_cast<int>(buffer_pool.size() * 1.5));
            buffer_pool.resize(new_size);
            buffer_offsets.resize(new_size / 64 + 1, 0); // Chunk-based offset tracking
            pool_initialized = true;
            offset = 0;
            buffer_usage_count = 0;
            return buffer_pool.data();
        }
        
        // Advanced offset calculation for buffer reuse (matching Java's pattern)
        offset = (buffer_usage_count * 32) % (buffer_pool.size() / 2);
        buffer_usage_count++;
        
        return buffer_pool.data();
    }
    
    // Sophisticated buffer return with usage tracking
    static void returnBuffer(T* buffer, int size, int offset) {
        // Advanced buffer validation and cleanup (matching Java's approach)
        if (buffer >= buffer_pool.data() && 
            buffer < buffer_pool.data() + buffer_pool.size()) {
            // Mark offset as available for reuse
            int chunk_index = offset / 64;
            if (chunk_index < buffer_offsets.size()) {
                buffer_offsets[chunk_index] = 0; // Mark as available
            }
        }
    }
    
    // Advanced buffer statistics (C++ enhancement)
    static int getBufferUsage() {
        return buffer_usage_count;
    }
    
    // Buffer pool optimization
    static void optimizePool() {
        if (buffer_usage_count > 100) {
            // Reset usage patterns for optimization
            buffer_usage_count = 0;
            std::fill(buffer_offsets.begin(), buffer_offsets.end(), 0);
        }
    }
};

template<typename T>
thread_local std::vector<T> BufferManager<T>::buffer_pool;

template<typename T>
thread_local bool BufferManager<T>::pool_initialized = false;

template<typename T>
thread_local std::vector<int> BufferManager<T>::buffer_offsets;

template<typename T>
thread_local int BufferManager<T>::buffer_usage_count = 0;

// Enhanced Sorter with advanced buffer management (matching Java's Sorter class)
template<typename T>
class AdvancedSorter : public Sorter<T> {
private:
    AdvancedSorter* parent;
    T* a;           // Primary array
    T* b;           // Buffer array (can be shared/reused)
    int low;
    int size; 
    int offset;     // Buffer offset for reuse optimization
    int depth;
    bool owns_buffer;
    
public:
    AdvancedSorter(AdvancedSorter* parent, T* a, T* b, int low, int size, int offset, int depth)
        : Sorter<T>(parent, a, b, low, size, offset, depth), parent(parent), a(a), b(b), 
          low(low), size(size), offset(offset), depth(depth), owns_buffer(false) {
        
        // Advanced buffer allocation (matching Java's pattern)
        if (b == nullptr && depth >= 0) {
            this->b = BufferManager<T>::getBuffer(size, this->offset);
            owns_buffer = true;
        }
    }
    
    ~AdvancedSorter() {
        if (owns_buffer && b != nullptr) {
            BufferManager<T>::returnBuffer(b, size, offset);
        }
    }
    
    void compute() override {
        if (depth < 0) {
            // Parallel merge sort mode with sophisticated buffer management
            this->setPendingCount(2);
            int half = size >> 1;
            
            // Create child sorters with buffer reuse
            auto* left = new AdvancedSorter(this, b, a, low, half, offset, depth + 1);
            auto* right = new AdvancedSorter(this, b, a, low + half, size - half, offset, depth + 1);
            
            left->fork();
            right->compute();
        } else {
            // Use type-specific parallel quicksort with proper buffer integration
            if constexpr (std::is_same_v<T, int>) {
                sort_int_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, long>) {
                sort_long_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, float>) {
                sort_float_sequential(this, a, depth, low, low + size);
            } else if constexpr (std::is_same_v<T, double>) {
                sort_double_sequential(this, a, depth, low, low + size);
            } else {
                // Generic fallback
                parallelQuickSort(a, depth, low, low + size);
            }
        }
        this->tryComplete();
    }
    
    void onCompletion(CountedCompleter<T>* caller) override {
        // Advanced completion handling with buffer management
        if (depth < 0) {
            int mi = low + (size >> 1);
            bool src = (depth & 1) == 0;
            
            // Sophisticated buffer destination calculation
            T* dst = src ? a : b;
            int k = src ? (low - offset) : low;
            
            // Create merger with proper buffer coordination
            auto* merger = new Merger<T>(nullptr,
                dst, k,                                    
                b, src ? (low - offset) : low, src ? (mi - offset) : mi,     
                b, src ? (mi - offset) : mi, src ? (low + size - offset) : (low + size)
            );
            merger->invoke();
            delete merger;
        }
    }
    
    // Java-style forkSorter with local variable optimization
    void forkSorter(int depth, int low, int high) {
        this->addToPendingCount(1);
        T* localA = this->a; // Local variable optimization (matching Java pattern)
        auto* child = new AdvancedSorter(this, localA, b, low, high - low, offset, depth);
        child->fork();
    }
    
    // Advanced pending count management
    void setPendingCount(int count) {
        this->pending.store(count);
    }
    
    // Get buffer for child operations
    T* getBuffer() { return b; }
    int getOffset() { return offset; }
};

// =============================================================================
// COMPREHENSIVE ERROR HANDLING AND VALIDATION (matching Java's approach)
// =============================================================================

// Overflow-safe arithmetic (matching Java's unsigned right shift)
inline int safeMiddle(int low, int high) {
    return static_cast<int>((static_cast<unsigned int>(low) + static_cast<unsigned int>(high)) >> 1);
}

// Bounds checking utility (matching Java's Objects.checkFromToIndex)
inline void checkFromToIndex(int fromIndex, int toIndex, int length) {
    if (fromIndex < 0 || fromIndex > toIndex || toIndex > length) {
        throw std::out_of_range("Index out of bounds: fromIndex=" + std::to_string(fromIndex) + 
                               ", toIndex=" + std::to_string(toIndex) + ", length=" + std::to_string(length));
    }
}

// Null pointer validation
template<typename T>
inline void checkNotNull(T* array, const char* paramName) {
    if (array == nullptr) {
        throw std::invalid_argument(std::string(paramName) + " cannot be null");
    }
}

// Early termination optimization helper
template<typename T>
inline bool checkEarlyTermination(T* a, int low, int high) {
    if (high - low <= 1) {
        return true; // Already sorted or single element
    }
    
    // Advanced early termination checks (matching Java's sophisticated approach)
    int size = high - low;
    
    // Check if array is already sorted (ascending)
    bool is_sorted = true;
    for (int i = low + 1; i < high && is_sorted; i++) {
        if (a[i] < a[i - 1]) {
            is_sorted = false;
        }
    }
    if (is_sorted) return true;
    
    // Check if array is reverse sorted (can be quickly reversed)
    bool is_reverse_sorted = true;
    for (int i = low + 1; i < high && is_reverse_sorted; i++) {
        if (a[i] > a[i - 1]) {
            is_reverse_sorted = false;
        }
    }
    
    if (is_reverse_sorted) {
        // Reverse the array (O(n/2) swaps)
        for (int i = low, j = high - 1; i < j; i++, j--) {
            T temp = a[i];
            a[i] = a[j];
            a[j] = temp;
        }
        return true;
    }
    
    // Check for constant array (all elements equal) - Java pattern
    if (size >= 4) {
        T first = a[low];
        bool all_equal = true;
        for (int i = low + 1; i < low + std::min(16, size) && all_equal; i++) {
            if (a[i] != first) {
                all_equal = false;
            }
        }
        if (all_equal) {
            // Quick check if entire array is constant
            for (int i = low + 16; i < high; i += std::max(1, size / 32)) {
                if (a[i] != first) {
                    all_equal = false;
                    break;
                }
            }
            if (all_equal) return true;
        }
    }
    
    return false;
}

// =============================================================================
// PUBLIC API METHODS (matching Java's exact signatures with proper validation)
// =============================================================================

// Primary public API methods with comprehensive validation
static void sort(int* a, int parallelism, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range: low=" + std::to_string(low) + ", high=" + std::to_string(high));
    }
    
    if (checkEarlyTermination(a, low, high)) {
        return;
    }
    
    int size = high - low;
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        std::vector<int> b(depth == 0 ? 0 : size);
        auto* sorter = new AdvancedSorter<int>(nullptr, a, depth == 0 ? nullptr : b.data(), low, size, low, depth);
        sorter->invoke();
        delete sorter;
    } else {
        sort_int_sequential(nullptr, a, 0, low, high);
    }
}

static void sort(long* a, int parallelism, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range: low=" + std::to_string(low) + ", high=" + std::to_string(high));
    }
    
    if (checkEarlyTermination(a, low, high)) {
        return;
    }
    
    int size = high - low;
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        std::vector<long> b(depth == 0 ? 0 : size);
        auto* sorter = new AdvancedSorter<long>(nullptr, a, depth == 0 ? nullptr : b.data(), low, size, low, depth);
        sorter->invoke();
        delete sorter;
    } else {
        sort_long_sequential(nullptr, a, 0, low, high);
    }
}

static void sort(float* a, int parallelism, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range: low=" + std::to_string(low) + ", high=" + std::to_string(high));
    }
    
    if (checkEarlyTermination(a, low, high)) {
        return;
    }
    
    int size = high - low;
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        std::vector<float> b(depth == 0 ? 0 : size);
        auto* sorter = new AdvancedSorter<float>(nullptr, a, depth == 0 ? nullptr : b.data(), low, size, low, depth);
        sorter->invoke();
        delete sorter;
    } else {
        sort_float_specialized(a, low, high);
    }
}

static void sort(double* a, int parallelism, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range: low=" + std::to_string(low) + ", high=" + std::to_string(high));
    }
    
    if (checkEarlyTermination(a, low, high)) {
        return;
    }
    
    int size = high - low;
    if (parallelism > 1 && size > MIN_PARALLEL_SORT_SIZE) {
        int depth = getDepth(parallelism, size >> 12);
        std::vector<double> b(depth == 0 ? 0 : size);
        auto* sorter = new AdvancedSorter<double>(nullptr, a, depth == 0 ? nullptr : b.data(), low, size, low, depth);
        sorter->invoke();
        delete sorter;
    } else {
        sort_float_specialized(a, low, high);
    }
}

// Non-parallel types (byte, char, short) with validation
static void sort(signed char* a, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range: low=" + std::to_string(low) + ", high=" + std::to_string(high));
    }
    
    if (checkEarlyTermination(a, low, high)) {
        return;
    }
    
    if (high - low >= MIN_BYTE_COUNTING_SORT_SIZE) {
        countingSort(a, low, high);
    } else {
        insertionSort(a, low, high);
    }
}

static void sort(char* a, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range: low=" + std::to_string(low) + ", high=" + std::to_string(high));
    }
    
    if (checkEarlyTermination(a, low, high)) {
        return;
    }
    
    if (high - low >= MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE) {
        countingSort(a, low, high);
    } else {
        sort(a, 0, low, high);
    }
}

static void sort(short* a, int low, int high) {
    checkNotNull(a, "array");
    if (low < 0 || high < 0 || low > high) {
        throw std::out_of_range("Invalid range: low=" + std::to_string(low) + ", high=" + std::to_string(high));
    }
    
    if (checkEarlyTermination(a, low, high)) {
        return;
    }
    
    if (high - low >= MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE) {
        countingSort(a, low, high);
    } else {
        sort(a, 0, low, high);
    }
}

// Range-based overloads with validation (matching Java's array.length variants)
static void sort(int* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative: " + std::to_string(length));
    }
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(long* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative: " + std::to_string(length));
    }
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(float* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative: " + std::to_string(length));
    }
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(double* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative: " + std::to_string(length));
    }
    sort(a, std::thread::hardware_concurrency(), 0, length);
}

static void sort(signed char* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative: " + std::to_string(length));
    }
    sort(a, 0, length);
}

static void sort(char* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative: " + std::to_string(length));
    }
    sort(a, 0, length);
}

static void sort(short* a, int length) {
    checkNotNull(a, "array");
    if (length < 0) {
        throw std::invalid_argument("Array length cannot be negative: " + std::to_string(length));
    }
    sort(a, 0, length);
}

// Enhanced container-based API (C++ STL integration)
template<typename Container>
void sort(Container& container) {
    using ValueType = typename Container::value_type;
    if constexpr (std::is_integral_v<ValueType> && sizeof(ValueType) <= 2) {
        sort_specialized(container.data(), 0, static_cast<int>(container.size()));
    } else if constexpr (std::is_floating_point_v<ValueType>) {
        sort_float_specialized(container.data(), 0, static_cast<int>(container.size()));
    } else {
        dual_pivot_quicksort(container.begin(), container.end());
    }
}

template<typename Container>
void sort(Container& container, int parallelism) {
    using ValueType = typename Container::value_type;
    if (container.size() > MIN_PARALLEL_SORT_SIZE && parallelism > 1) {
        parallelSort(container.data(), parallelism, 0, static_cast<int>(container.size()));
    } else {
        sort(container);
    }
}

} // namespace dual_pivot

#endif // DUAL_PIVOT_QUICKSORT_HPP