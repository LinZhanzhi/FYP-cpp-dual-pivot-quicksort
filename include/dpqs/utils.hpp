#ifndef DPQS_UTILS_HPP
#define DPQS_UTILS_HPP

#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdint>
#include <utility>
#include <stdexcept>
#include <string>
#include <iterator>
#include <type_traits>

// Compiler optimization hints
#if defined(__GNUC__) || defined(__clang__)
    #define DPQS_FORCE_INLINE __attribute__((always_inline)) inline
    #define DPQS_LIKELY(x) __builtin_expect(!!(x), 1)
    #define DPQS_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define DPQS_PREFETCH_READ(ptr) __builtin_prefetch((ptr), 0, 3)
    #define DPQS_PREFETCH_WRITE(ptr) __builtin_prefetch((ptr), 1, 3)
#elif defined(_MSC_VER)
    #define DPQS_FORCE_INLINE __forceinline
    #define DPQS_LIKELY(x) (x)
    #define DPQS_UNLIKELY(x) (x)
    #define DPQS_PREFETCH_READ(ptr)
    #define DPQS_PREFETCH_WRITE(ptr)
#else
    #define DPQS_FORCE_INLINE inline
    #define DPQS_LIKELY(x) (x)
    #define DPQS_UNLIKELY(x) (x)
    #define DPQS_PREFETCH_READ(ptr)
    #define DPQS_PREFETCH_WRITE(ptr)
#endif

// Constants
constexpr std::ptrdiff_t MAX_RECURSION_DEPTH = 64;
constexpr std::ptrdiff_t INSERTION_SORT_THRESHOLD = 32;
constexpr std::ptrdiff_t MIXED_INSERTION_SORT_THRESHOLD = 48;
constexpr std::ptrdiff_t MAX_RUN_CAPACITY = 500;
constexpr std::ptrdiff_t MIN_FIRST_RUN_SIZE = 16;
constexpr std::ptrdiff_t MIN_RUN_COUNT = 5;
constexpr std::ptrdiff_t MIN_BYTE_COUNTING_SORT_SIZE = 64;
constexpr std::ptrdiff_t MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE = 1750;
constexpr std::ptrdiff_t MAX_MIXED_INSERTION_SORT_SIZE = 48;
constexpr std::ptrdiff_t MAX_INSERTION_SORT_SIZE = 32;
constexpr std::ptrdiff_t MIN_TRY_MERGE_SIZE = 64;
constexpr std::ptrdiff_t DELTA = 3; // Recursion depth delta
constexpr std::ptrdiff_t MIN_PARALLEL_SORT_SIZE = 4096; // Threshold for parallel sorting

namespace dual_pivot {

// Trait to detect contiguous iterators
template<typename Iter, typename = void>
struct is_contiguous_iterator : std::false_type {};

template<typename Iter>
struct is_contiguous_iterator<Iter, std::enable_if_t<std::is_pointer_v<Iter>>> : std::true_type {};

#if __cplusplus >= 202002L
template<typename Iter>
struct is_contiguous_iterator<Iter, std::enable_if_t<std::contiguous_iterator<Iter>>> : std::true_type {};
#endif

template<typename Iter>
constexpr bool is_contiguous_iterator_v = is_contiguous_iterator<Iter>::value;

// Utility functions
template<typename T>
DPQS_FORCE_INLINE void swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}

template<typename T>
void checkNotNull(T* ptr, const std::string& name) {
    if (ptr == nullptr) {
        throw std::invalid_argument(name + " must not be null");
    }
}

template<typename T, typename Compare>
bool checkEarlyTermination(T* a, std::ptrdiff_t low, std::ptrdiff_t high, Compare comp) {
    if (high - low <= 1) return true;
    for (std::ptrdiff_t i = low; i < high - 1; i++) {
        if (comp(a[i+1], a[i])) return false;
    }
    return true;
}

template<typename T>
bool checkEarlyTermination(T* a, std::ptrdiff_t low, std::ptrdiff_t high) {
    return checkEarlyTermination(a, low, high, std::less<T>());
}

inline int getDepth(int parallelism, std::ptrdiff_t size_factor) {
    if (parallelism <= 1) return 0;
    return static_cast<int>(std::ceil(std::log2(parallelism))) + 1;
}

inline std::ptrdiff_t safeMiddle(std::ptrdiff_t low, std::ptrdiff_t high) {
    return (low + high) >> 1;
}

inline void checkFromToIndex(std::ptrdiff_t fromIndex, std::ptrdiff_t toIndex, std::ptrdiff_t length) {
    if (fromIndex < 0 || fromIndex > toIndex || toIndex > length) {
        throw std::out_of_range("Index out of bounds");
    }
}

}

#endif // DPQS_UTILS_HPP
