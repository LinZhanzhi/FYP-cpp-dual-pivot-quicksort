/**
 * @file dual_pivot_quicksort.hpp
 * @brief Comprehensive C++ implementation of Vladimir Yaroslavskiy's dual-pivot quicksort algorithm
 *
 * This implementation is based on the dual-pivot quicksort algorithm that was adopted in Java 7
 * and significantly outperforms traditional single-pivot quicksort variants. The algorithm uses
 * two pivot elements to create three-way partitioning, reducing the average number of element
 * swaps from 1.0×n×ln(n) to 0.8×n×ln(n) compared to classic quicksort.
 *
 * Key Performance Benefits (from Sebastian Wild's research):
 * - 20% fewer swaps than traditional quicksort
 * - 12% fewer "scanned elements" (memory accesses), crucial for modern CPU-memory performance gaps
 * - Better cache locality due to optimized memory access patterns
 * - Superior performance on arrays with many duplicate elements
 *
 * The implementation includes:
 * - STL-compatible iterator interface
 * - Advanced optimizations: introsort-style depth limiting, run detection and merging
 * - Parallel processing support with sophisticated work-stealing patterns
 * - Type-specific optimizations for primitive types (int, long, float, double, byte, char, short)
 * - Special handling for floating-point edge cases (NaN, negative zero)
 * - Comprehensive error handling and validation
 *
 * @author Implementation based on Vladimir Yaroslavskiy's dual-pivot quicksort
 * @version C++ port with advanced optimizations and parallel support
 */

#ifndef DPQS_UTILS_HPP
#define DPQS_UTILS_HPP
#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define PREFETCH_READ(ptr) __builtin_prefetch(ptr, 0, 3)
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#define PREFETCH_READ(ptr) ((void)0)
#endif


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

#if __cplusplus >= 202002L
#include <concepts>
#endif

// #include "dpqs/utils.hpp"
#include "dpqs/constants.hpp"

/**
 * @brief Main namespace containing the dual-pivot quicksort implementation
 *
 * This namespace encapsulates all components of the dual-pivot quicksort algorithm,
 * including the core sorting functions, helper utilities, and optimization classes.
 * The design closely follows the Java implementation while adding C++-specific
 * enhancements for performance and type safety.
 */
namespace dual_pivot {

#if __cplusplus >= 202002L
    template<typename T>
    concept Integral = std::is_integral_v<T>;

    template<typename T>
    concept FloatingPoint = std::is_floating_point_v<T>;

    template<typename T>
    concept Primitive = Integral<T> || FloatingPoint<T>;
#endif

// =============================================================================
// OBJECT-BASED GENERIC ARRAY HANDLING (matching Java's Object a, b pattern)
// =============================================================================

/**
 * @brief Type-erased array variant for generic array operations
 *
 * This variant type allows the algorithm to work with different primitive types
 * in a type-safe manner, similar to Java's Object[] arrays. It supports all
 * primitive types that benefit from dual-pivot quicksort optimizations.
 *
 * The variant approach provides better type safety than void* while maintaining
 * the flexibility needed for generic array operations in the sorting implementation.
 */
using ArrayVariant = std::variant<
    int*, long*, float*, double*,           // Main numeric types
    signed char*, char*, short*,            // Smaller integer types
    unsigned char*, unsigned short*        // Unsigned variants
>;

/**
 * @brief Array pointer wrapper for type-safe array operations
 *
 * This wrapper class provides a type-safe interface for working with arrays of different
 * primitive types, similar to Java's Object array handling but with compile-time type safety.
 * It encapsulates the array pointer along with size and element size information.
 *
 * The class provides type checking methods (equivalent to Java's instanceof) and
 * type extraction methods (equivalent to Java's casting) to enable generic algorithms
 * while maintaining type safety.
 */
struct ArrayPointer {
    ArrayVariant data;      ///< The actual array pointer stored as a variant
    int size;              ///< Number of elements in the array
    int element_size;      ///< Size of each element in bytes

    /**
     * @brief Default constructor creating an empty ArrayPointer
     */
    ArrayPointer() : data(static_cast<int*>(nullptr)), size(0), element_size(0) {}

    /**
     * @brief Template constructor for creating ArrayPointer from typed array
     * @tparam T The element type of the array
     * @param ptr Pointer to the array
     * @param sz Size of the array (default: 0)
     */
    template<typename T>
    ArrayPointer(T* ptr, int sz = 0) : data(ptr), size(sz), element_size(sizeof(T)) {}

    // Type checking methods (equivalent to Java's instanceof operator)

    /**
     * @brief Check if the array contains int elements
     * @return true if the array is of type int*
     */
    bool isIntArray() const { return std::holds_alternative<int*>(data); }

    /**
     * @brief Check if the array contains long elements
     * @return true if the array is of type long*
     */
    bool isLongArray() const { return std::holds_alternative<long*>(data); }

    /**
     * @brief Check if the array contains float elements
     * @return true if the array is of type float*
     */
    bool isFloatArray() const { return std::holds_alternative<float*>(data); }

    /**
     * @brief Check if the array contains double elements
     * @return true if the array is of type double*
     */
    bool isDoubleArray() const { return std::holds_alternative<double*>(data); }

    /**
     * @brief Check if the array contains signed char elements
     * @return true if the array is of type signed char*
     */
    bool isByteArray() const { return std::holds_alternative<signed char*>(data); }

    /**
     * @brief Check if the array contains char elements
     * @return true if the array is of type char*
     */
    bool isCharArray() const { return std::holds_alternative<char*>(data); }

    /**
     * @brief Check if the array contains short elements
     * @return true if the array is of type short*
     */
    bool isShortArray() const { return std::holds_alternative<short*>(data); }

    /**
     * @brief Check if the array contains unsigned char elements
     * @return true if the array is of type unsigned char*
     */
    bool isUnsignedByteArray() const { return std::holds_alternative<unsigned char*>(data); }

    /**
     * @brief Check if the array contains unsigned short elements
     * @return true if the array is of type unsigned short*
     */
    bool isUnsignedShortArray() const { return std::holds_alternative<unsigned short*>(data); }

    // Type extraction methods (equivalent to Java's casting operations)

    /**
     * @brief Extract int array pointer
     * @return Pointer to int array
     * @throws std::bad_variant_access if the array is not of type int*
     */
    int* asIntArray() const { return std::get<int*>(data); }

    /**
     * @brief Extract long array pointer
     * @return Pointer to long array
     * @throws std::bad_variant_access if the array is not of type long*
     */
    long* asLongArray() const { return std::get<long*>(data); }

    /**
     * @brief Extract float array pointer
     * @return Pointer to float array
     * @throws std::bad_variant_access if the array is not of type float*
     */
    float* asFloatArray() const { return std::get<float*>(data); }

    /**
     * @brief Extract double array pointer
     * @return Pointer to double array
     * @throws std::bad_variant_access if the array is not of type double*
     */
    double* asDoubleArray() const { return std::get<double*>(data); }

    /**
     * @brief Extract signed char array pointer
     * @return Pointer to signed char array
     * @throws std::bad_variant_access if the array is not of type signed char*
     */
    signed char* asByteArray() const { return std::get<signed char*>(data); }

    /**
     * @brief Extract char array pointer
     * @return Pointer to char array
     * @throws std::bad_variant_access if the array is not of type char*
     */
    char* asCharArray() const { return std::get<char*>(data); }

    /**
     * @brief Extract short array pointer
     * @return Pointer to short array
     * @throws std::bad_variant_access if the array is not of type short*
     */
    short* asShortArray() const { return std::get<short*>(data); }

    /**
     * @brief Extract unsigned char array pointer
     * @return Pointer to unsigned char array
     * @throws std::bad_variant_access if the array is not of type unsigned char*
     */
    unsigned char* asUnsignedByteArray() const { return std::get<unsigned char*>(data); }

    /**
     * @brief Extract unsigned short array pointer
     * @return Pointer to unsigned short array
     * @throws std::bad_variant_access if the array is not of type unsigned short*
     */
    unsigned short* asUnsignedShortArray() const { return std::get<unsigned short*>(data); }

    /**
     * @brief Generic visitor pattern for type dispatch
     *
     * This method allows applying operations to the stored array regardless of its type
     * by using the visitor pattern. The visitor function will be called with the actual
     * typed pointer.
     *
     * @tparam Visitor A callable that can handle all possible array types
     * @param visitor The visitor function to apply
     * @return The result of the visitor function
     */
    template<typename Visitor>
    auto visit(Visitor&& visitor) const {
        return std::visit(std::forward<Visitor>(visitor), data);
    }
};

/**
 * @brief Factory function for creating ArrayPointer instances
 *
 * Convenience function to create ArrayPointer objects with proper type deduction.
 *
 * @tparam T The element type of the array
 * @param ptr Pointer to the array
 * @param size Size of the array (default: 0)
 * @return ArrayPointer wrapping the given array
 */
template<typename T>
ArrayPointer makeArrayPointer(T* ptr, int size = 0) {
    return ArrayPointer(ptr, size);
}


/**
 * @brief C++ equivalent of Java's Float.floatToRawIntBits()
 *
 * Converts a float value to its raw IEEE 754 bit representation without
 * any modifications. This is essential for handling special floating-point
 * values like NaN and negative zero in sorting algorithms.
 *
 * Uses std::bit_cast in C++20 for optimal performance, falls back to
 * memcpy for older standards while maintaining strict aliasing compliance.
 *
 * @param value The float value to convert
 * @return The raw bit representation as uint32_t
 */
inline std::uint32_t floatToRawIntBits(float value) {
#if __cpp_lib_bit_cast >= 201806L
    return std::bit_cast<std::uint32_t>(value);
#else
    // Fallback for pre-C++20 compilers - maintains strict aliasing compliance
    static_assert(sizeof(float) == sizeof(std::uint32_t), "Float and uint32_t must have same size");
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(float));
    return result;
#endif
}

/**
 * @brief C++ equivalent of Java's Double.doubleToRawLongBits()
 *
 * Converts a double value to its raw IEEE 754 bit representation without
 * any modifications. Critical for proper handling of special floating-point
 * values in sorting operations.
 *
 * @param value The double value to convert
 * @return The raw bit representation as uint64_t
 */
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

/**
 * @brief C++ equivalent of Java's Float.intBitsToFloat()
 *
 * Converts a raw bit pattern back to a float value. Used to reconstruct
 * floating-point values from their bit representations, particularly for
 * restoring special values after sorting operations.
 *
 * @param bits The bit pattern to convert
 * @return The float value represented by the bits
 */
inline float intBitsToFloat(std::uint32_t bits) {
#if __cpp_lib_bit_cast >= 201806L
    return std::bit_cast<float>(bits);
#else
    float result;
    std::memcpy(&result, &bits, sizeof(float));
    return result;
#endif
}

/**
 * @brief C++ equivalent of Java's Double.longBitsToDouble()
 *
 * Converts a raw bit pattern back to a double value. Essential for
 * reconstructing double values from their bit representations.
 *
 * @param bits The bit pattern to convert
 * @return The double value represented by the bits
 */
inline double longBitsToDouble(std::uint64_t bits) {
#if __cpp_lib_bit_cast >= 201806L
    return std::bit_cast<double>(bits);
#else
    double result;
    std::memcpy(&result, &bits, sizeof(double));
    return result;
#endif
}

/**
 * @brief Enhanced negative zero detection using precise bit manipulation
 *
 * Detects negative zero (-0.0f) by examining the sign bit directly.
 * This is crucial for maintaining IEEE 754 compliance in sorting algorithms
 * where -0.0 should be treated differently from +0.0.
 *
 * @param value The float value to check
 * @return true if the value is negative zero
 */
inline bool isNegativeZero(float value) {
    return value == 0.0f && floatToRawIntBits(value) == 0x80000000U;
}

/**
 * @brief Enhanced negative zero detection for double precision
 *
 * Detects negative zero (-0.0) in double precision floating-point values.
 *
 * @param value The double value to check
 * @return true if the value is negative zero
 */
inline bool isNegativeZero(double value) {
    return value == 0.0 && doubleToRawLongBits(value) == 0x8000000000000000ULL;
}

/**
 * @brief Optimized NaN detection using bit patterns
 *
 * Detects NaN (Not a Number) values by examining the IEEE 754 bit pattern
 * directly. This is more reliable than using comparison operations, which
 * may be optimized away by compilers.
 *
 * NaN bit pattern for float: exponent = 0x7F800000, mantissa != 0
 *
 * @param value The float value to check
 * @return true if the value is NaN
 */
inline bool isNaN(float value) {
    std::uint32_t bits = floatToRawIntBits(value);
    return (bits & 0x7F800000U) == 0x7F800000U && (bits & 0x007FFFFFU) != 0;
}

/**
 * @brief Optimized NaN detection for double precision
 *
 * Detects NaN values in double precision floating-point numbers using
 * direct bit pattern examination.
 *
 * NaN bit pattern for double: exponent = 0x7FF0000000000000, mantissa != 0
 *
 * @param value The double value to check
 * @return true if the value is NaN
 */
inline bool isNaN(double value) {
    std::uint64_t bits = doubleToRawLongBits(value);
    return (bits & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL && (bits & 0x000FFFFFFFFFFFFFULL) != 0;
}

/**
 * @brief Precise positive zero detection using bit patterns
 *
 * Detects positive zero (+0.0f) by examining the bit representation directly.
 * This distinguishes between +0.0 and -0.0, which is important for IEEE 754
 * compliant sorting.
 *
 * @param value The float value to check
 * @return true if the value is positive zero
 */
inline bool isPositiveZero(float value) {
    return floatToRawIntBits(value) == 0x00000000U;
}

/**
 * @brief Precise positive zero detection for double precision
 *
 * Detects positive zero (+0.0) in double precision values.
 *
 * @param value The double value to check
 * @return true if the value is positive zero
 */
inline bool isPositiveZero(double value) {
    return doubleToRawLongBits(value) == 0x0000000000000000ULL;
}

/**
 * @brief Advanced binary search for zero position restoration
 *
 * This function performs a binary search to find the position where zeros should
 * be inserted in a sorted array. It uses unsigned right shift operations to match
 * Java's >>> operator semantics, ensuring identical behavior across platforms.
 *
 * This is used during floating-point sorting to restore the proper positions of
 * negative zeros after the main sorting phase, maintaining IEEE 754 compliance.
 *
 * @tparam T The element type (typically float or double)
 * @param a Pointer to the sorted array
 * @param low Starting index for the search
 * @param high Ending index for the search
 * @return The position where zeros should be inserted
 */
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

/**
 * @brief Functional interface architecture for flexible algorithm dispatch
 *
 * This section implements a functional interface system similar to Java's method
 * references and lambda expressions. It allows the dual-pivot quicksort algorithm
 * to be parameterized with different sorting strategies, partitioning methods,
 * and optimization techniques while maintaining type safety and performance.
 *
 * The approach enables:
 * - Runtime algorithm selection based on data characteristics
 * - Easy testing of different optimization strategies
 * - Modular algorithm composition for hybrid approaches
 * - Performance comparison between different implementations
 */

// Forward declarations for function pointer types
/**
 * @brief Optimized function dispatch for partitioning operations
 *
 * Intrinsic candidate for partitioning operations, providing efficient dispatch
 * while maintaining the benefits of parameterized algorithm selection.
 *
 * @tparam T The element type being partitioned
 * @param array Pointer to the array to partition
 * @param low Starting index
 * @param high Ending index
 * @param pivotIndex1 First pivot index
 * @param pivotIndex2 Second pivot index
 * @param po The partitioning operation to execute
 * @return Partition boundaries as returned by the operation
 */

template<typename T>
using SortOperation = void(*)(T* a, int low, int high);

template<typename T>
FORCE_INLINE void sort_intrinsic(T* array, int low, int high, SortOperation<T> so) {
    so(array, low, high);
}

template<typename T>
using PartitionOperation = std::pair<int, int>(*)(T* a, int low, int high, int pivotIndex1, int pivotIndex2);

template<typename T>
FORCE_INLINE std::pair<int, int> partition_intrinsic(T* array, int low, int high,
                                                     int pivotIndex1, int pivotIndex2,
                                                     PartitionOperation<T> po) {
    return po(array, low, high, pivotIndex1, pivotIndex2);
}

/**
 * @brief Forward declarations for algorithm method references
 *
 * These forward declarations define the available sorting and partitioning
 * algorithms that can be used as method references in the functional interface.
 * Each corresponds to a specific optimization strategy or algorithm variant.
 */
template<typename T> void insertionSort_ref(T* a, int low, int high);
template<typename T> void mixedInsertionSort_ref(T* a, int low, int high);
template<typename T> void heapSort_ref(T* a, int low, int high);
template<typename T> std::pair<int, int> partitionDualPivot_ref(T* a, int low, int high, int pivotIndex1, int pivotIndex2);
template<typename T> std::pair<int, int> partitionSinglePivot_ref(T* a, int low, int high, int pivotIndex1, int pivotIndex2);



/**
 * @brief Thread pool for parallel sorting operations
 *
 * This thread pool implementation provides work distribution for parallel
 * sorting algorithms. It uses a producer-consumer pattern with condition
 * variables for efficient thread coordination and work stealing.
 *
 * The design follows modern C++ best practices for thread management and
 * provides RAII-style automatic cleanup. Tasks are executed asynchronously
 * with future-based result handling.
 *
 * Key features:
 * - Automatic thread count detection based on hardware capabilities
 * - Efficient task queuing with condition variable synchronization
 * - Future-based result handling for async operations
 * - RAII-style cleanup with proper thread joining
 */
class ThreadPool {
private:
    std::vector<std::thread> workers;           ///< Worker threads
    std::queue<std::function<void()>> tasks;   ///< Task queue
    std::mutex queue_mutex;                     ///< Mutex for thread-safe queue access
    std::condition_variable condition;          ///< Condition variable for worker synchronization
    std::atomic<bool> stop;                     ///< Atomic flag for clean shutdown

public:
    /**
     * @brief Construct a thread pool with specified number of threads
     *
     * Creates a thread pool with the given number of worker threads. Each worker
     * runs in a loop, waiting for tasks to be enqueued and executing them.
     *
     * @param num_threads Number of worker threads (default: hardware concurrency)
     */
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

    /**
     * @brief Enqueue a task for asynchronous execution
     *
     * Adds a callable task to the thread pool's work queue. The task will be
     * executed by one of the worker threads when available. Returns a future
     * that can be used to retrieve the result.
     *
     * @tparam F Function type
     * @tparam Args Argument types
     * @param f Function to execute
     * @param args Arguments to pass to the function
     * @return std::future for retrieving the result
     * @throws std::runtime_error if the thread pool has been stopped
     */
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

    /**
     * @brief Destructor - ensures clean shutdown of all threads
     *
     * Signals all worker threads to stop, wakes them up, and waits for
     * them to complete their current tasks before destruction.
     */
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

/**
 * @brief Singleton thread pool accessor for parallel sorting
 *
 * Provides a global thread pool instance for use in parallel sorting operations.
 * The singleton pattern ensures that only one thread pool exists per process,
 * avoiding resource waste and thread proliferation.
 *
 * The thread pool is created with the default hardware concurrency and remains
 * active for the lifetime of the program.
 *
 * @return Reference to the global ThreadPool instance
 */
static ThreadPool& getThreadPool() {
    static ThreadPool pool;
    return pool;
}

// =============================================================================
// =============================================================================
// MERGE OPERATIONS (move before parallel classes to fix instantiation issues)
// =============================================================================

/**
 * @brief Sequential merge operation for combining two sorted array segments
 *
 * This function merges two sorted array segments into a destination array.
 * It uses the standard two-pointer merge technique optimized for cache performance.
 * The implementation includes buffer management optimizations to handle cases
 * where the destination overlaps with source arrays.
 *
 * Algorithm: Standard merge with three phases:
 * 1. Merge while both arrays have elements (main merge loop)
 * 2. Copy remaining elements from first array if any
 * 3. Copy remaining elements from second array if any
 *
 * Buffer Management:
 * - Handles overlapping destination and source arrays safely
 * - Optimizes for cases where destination is the same as source
 * - Prevents unnecessary copying when arrays don't overlap
 *
 * Time Complexity: O(n + m) where n and m are the sizes of the two segments
 * Space Complexity: O(1) additional space
 *
 * @tparam T Element type (must support comparison and assignment)
 * @param dst Destination array for merged result
 * @param k Starting index in destination array
 * @param a1 First source array
 * @param lo1 Starting index of first segment (inclusive)
 * @param hi1 Ending index of first segment (exclusive)
 * @param a2 Second source array
 * @param lo2 Starting index of second segment (inclusive)
 * @param hi2 Ending index of second segment (exclusive)
 */
template<typename T>
void mergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2) {
    // Phase 1: Main merge loop - process both arrays while they have elements
    // Uses branch-free comparison for better performance on modern CPUs
    while (lo1 < hi1 && lo2 < hi2) {
        dst[k++] = (a1[lo1] < a2[lo2]) ? a1[lo1++] : a2[lo2++];
    }

    // Phase 2: Copy remaining elements from first array
    // Buffer overlap check prevents unnecessary copying when dst == a1
    if (dst != a1 || k < lo1) {
        while (lo1 < hi1) {
            dst[k++] = a1[lo1++];
        }
    }

    // Phase 3: Copy remaining elements from second array
    // Buffer overlap check prevents unnecessary copying when dst == a2
    if (dst != a2 || k < lo2) {
        while (lo2 < hi2) {
            dst[k++] = a2[lo2++];
        }
    }
}

/**
 * @brief Parallel merge operation using divide-and-conquer with binary search
 *
 * This function implements a parallel merge algorithm that recursively divides
 * large merge operations into smaller subproblems that can be processed concurrently.
 * It uses binary search to find optimal split points that balance workload.
 *
 * Algorithm Strategy:
 * 1. Check if both segments are large enough for parallel processing
 * 2. Ensure the first array is the larger one (swap if necessary)
 * 3. Find median element of larger array and binary search position in smaller array
 * 4. Launch parallel tasks for the two resulting merge operations
 * 5. Fall back to sequential merge for small segments
 *
 * Parallel Subdivision:
 * - Uses binary search (std::lower_bound) to find split points
 * - Ensures balanced workload distribution between threads
 * - Maintains cache locality by processing related data together
 *
 * Load Balancing:
 * - Always makes the larger array the primary partitioning source
 * - Uses median split to ensure roughly equal work distribution
 * - Recursive subdivision continues until segments are too small for parallelism
 *
 * Time Complexity: O(log(n+m)) depth with O(n+m) total work
 * Space Complexity: O(log(n+m)) recursion stack space
 *
 * @tparam T Element type (must support comparison and assignment)
 * @param dst Destination array for merged result
 * @param k Starting index in destination array
 * @param a1 First source array
 * @param lo1 Starting index of first segment (inclusive)
 * @param hi1 Ending index of first segment (exclusive)
 * @param a2 Second source array
 * @param lo2 Starting index of second segment (inclusive)
 * @param hi2 Ending index of second segment (exclusive)
 */
template<typename T>
void parallelMergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2) {
    // Check if both segments are large enough for parallel processing
    if (hi1 - lo1 >= MIN_PARALLEL_MERGE_PARTS_SIZE && hi2 - lo2 >= MIN_PARALLEL_MERGE_PARTS_SIZE) {
        // Ensure first array is larger for optimal partitioning
        // This load balancing step ensures the binary search is performed on the smaller array
        if (hi1 - lo1 < hi2 - lo2) {
            std::swap(lo1, lo2);
            std::swap(hi1, hi2);
            std::swap(a1, a2);
        }

        // Find median of larger array for balanced workload distribution
        int mi1 = (lo1 + hi1) >> 1;
        T key = a1[mi1];

        // Binary search to find split point in smaller array
        // This ensures elements < key go to left merge, elements >= key go to right merge
        int mi2 = std::lower_bound(a2 + lo2, a2 + hi2, key) - a2;

        // Calculate destination offset for right merge operation
        int d = mi2 - lo2 + mi1 - lo1;

        // Launch parallel task for right partition
        auto& pool = getThreadPool();
        auto future = pool.enqueue([=] {
            parallelMergeParts(dst, k + d, a1, mi1, hi1, a2, mi2, hi2);
        });

        // Process left partition in current thread
        parallelMergeParts(dst, k, a1, lo1, mi1, a2, lo2, mi2);

        // Wait for right partition to complete
        future.get();
    } else {
        // Fall back to sequential merge for small segments
        // This avoids thread creation overhead for small workloads
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

/**
 * @brief Advanced parallel processing framework (matching Java's CountedCompleter)
 *
 * This section implements a sophisticated parallel coordination system based on
 * Java's CountedCompleter framework. The design enables fine-grained parallel
 * task decomposition with automatic load balancing and completion propagation.
 *
 * Key Components:
 * - CountedCompleter: Base class for fork-join style parallel tasks
 * - Pending count management: Tracks outstanding child tasks
 * - Exception propagation: Handles errors across task boundaries
 * - Completion callbacks: Enables complex coordination patterns
 *
 * Design Patterns:
 * - Fork-join parallelism: Recursive task subdivision
 * - Work stealing: Dynamic load balancing across threads
 * - Completion trees: Hierarchical task dependency management
 * - Exception handling: Graceful error propagation and recovery
 */

// Forward declarations for advanced parallel classes
// Generic Merger class for Object-based array merging (matching Java's Merger extends CountedCompleter)


/**
 * @brief Parallel merger for combining sorted array segments
 *
 * This class implements parallel merging of two sorted array segments using
 * recursive decomposition. It automatically switches between parallel and
 * sequential merging based on segment sizes to optimize performance.
 *
 * Parallel Merge Strategy:
 * - Large segments: Use binary search partitioning for parallel subdivision
 * - Small segments: Use sequential merge to avoid thread overhead
 * - Load balancing: Ensure work is distributed evenly across threads
 *
 * Binary Search Partitioning:
 * - Find split points using std::lower_bound for balanced workload
 * - Recursive subdivision until segments are too small for parallelism
 * - Cache-aware processing to minimize memory access overhead
 *
 * @tparam T Element type being merged
 */


// Forward declarations for type-specific functions
template<typename T> void insertionSort_ref(T* a, int low, int high);
template<typename T> void mixedInsertionSort_ref(T* a, int low, int high);
template<typename T> void heapSort_ref(T* a, int low, int high);
template<typename T> std::pair<int, int> partitionDualPivot_ref(T* a, int low, int high, int pivotIndex1, int pivotIndex2);
template<typename T> std::pair<int, int> partitionSinglePivot_ref(T* a, int low, int high, int pivotIndex1, int pivotIndex2);

// Type-specific function declarations








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
#if __cplusplus >= 202002L
template<FloatingPoint T>
void sort_specialized(T* a, int low, int high);
#else
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
sort_specialized(T* a, int low, int high);
#endif

// Forward declaration for merge operations
template<typename T>
void mergeParts(T* dst, int k, T* a1, int lo1, int hi1, T* a2, int lo2, int hi2);

/**
 * @brief Cache-friendly insertion sort with prefetching optimizations
 *
 * This is an optimized implementation of insertion sort that serves as the base case
 * for small arrays in the dual-pivot quicksort algorithm. It includes several
 * performance optimizations based on modern CPU characteristics:
 *
 * Key optimizations:
 * - Memory prefetching to reduce cache misses
 * - Branch prediction hints to reduce pipeline stalls
 * - Optimized inner loop for better instruction scheduling
 *
 * The algorithm threshold is carefully tuned: arrays smaller than MAX_INSERTION_SORT_SIZE
 * (44 elements) benefit from this approach over more complex algorithms.
 *
 * Time complexity: O(n²) in worst case, O(n) for nearly sorted data
 * Space complexity: O(1)
 *
 * @tparam T Element type (must support comparison and assignment)
 * @param a Pointer to the array to sort
 * @param low Starting index (inclusive)
 * @param high Ending index (exclusive)
 */

template<typename T>
inline void checkNotNull(T* ptr, const char* msg) {
    if (ptr == nullptr) {
        throw std::invalid_argument(std::string(msg) + " cannot be null");
    }
}

template<typename T>
inline bool checkEarlyTermination(T* a, int low, int high) {
    if (low >= high) return true;
    return false;
}

inline int getDepth(int parallelism, int size_factor) {
    if (parallelism <= 1) return 0;
    int depth = 0;
    while (parallelism > 0) {
        parallelism >>= 1;
        depth++;
    }
    return depth;
}

} // namespace dual_pivot

#endif // DPQS_UTILS_HPP
