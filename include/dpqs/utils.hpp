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
    #define FORCE_INLINE __attribute__((always_inline)) inline
    #define LIKELY(x) __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define PREFETCH_READ(ptr) __builtin_prefetch((ptr), 0, 3)
    #define PREFETCH_WRITE(ptr) __builtin_prefetch((ptr), 1, 3)
#elif defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
    #define PREFETCH_READ(ptr)
    #define PREFETCH_WRITE(ptr)
#else
    #define FORCE_INLINE inline
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
    #define PREFETCH_READ(ptr)
    #define PREFETCH_WRITE(ptr)
#endif

// Constants
constexpr int MAX_RECURSION_DEPTH = 64;
constexpr int INSERTION_SORT_THRESHOLD = 44;
constexpr int MIXED_INSERTION_SORT_THRESHOLD = 65;
constexpr int MAX_RUN_CAPACITY = 500;
constexpr int MIN_FIRST_RUN_SIZE = 16;
constexpr int MIN_RUN_COUNT = 5;
constexpr int MIN_BYTE_COUNTING_SORT_SIZE = 64;
constexpr int MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE = 1750;
constexpr int MAX_MIXED_INSERTION_SORT_SIZE = 65;
constexpr int MAX_INSERTION_SORT_SIZE = 44;
constexpr int MIN_TRY_MERGE_SIZE = 64;
constexpr int DELTA = 3; // Recursion depth delta
constexpr int MIN_PARALLEL_SORT_SIZE = 4096; // Threshold for parallel sorting

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
FORCE_INLINE void swap(T& a, T& b) {
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

template<typename T>
bool checkEarlyTermination(T* a, int low, int high) {
    if (high - low <= 1) return true;
    for (int i = low; i < high - 1; i++) {
        if (a[i] > a[i+1]) return false;
    }
    return true;
}

inline int getDepth(int parallelism, int size_factor) {
    if (parallelism <= 1) return 0;
    return static_cast<int>(std::ceil(std::log2(parallelism))) + 1;
}

inline int safeMiddle(int low, int high) {
    return static_cast<int>((static_cast<unsigned int>(low) + static_cast<unsigned int>(high)) >> 1);
}

inline void checkFromToIndex(int fromIndex, int toIndex, int length) {
    if (fromIndex < 0 || fromIndex > toIndex || toIndex > length) {
        throw std::out_of_range("Index out of bounds");
    }
}

}

#endif // DPQS_UTILS_HPP
