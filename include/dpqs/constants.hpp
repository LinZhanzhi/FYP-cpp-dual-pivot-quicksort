#ifndef DPQS_CONSTANTS_HPP
#define DPQS_CONSTANTS_HPP

namespace dual_pivot {

// Constants with #ifndef guards to allow compiler flag overrides

#ifndef MAX_INSERTION_SORT_SIZE
constexpr int MAX_INSERTION_SORT_SIZE = 60;
#endif

#ifndef MIN_PARALLEL_SORT_SIZE
constexpr int MIN_PARALLEL_SORT_SIZE = 65536;
#endif

#ifndef MAX_RUN_COUNT
constexpr int MAX_RUN_COUNT = 67;
#endif

#ifndef MAX_RUN_LENGTH
constexpr int MAX_RUN_LENGTH = 33;
#endif

#ifndef QUICKSORT_THRESHOLD
constexpr int QUICKSORT_THRESHOLD = 286;
#endif

#ifndef COUNTING_SORT_THRESHOLD_BYTE
constexpr int COUNTING_SORT_THRESHOLD_BYTE = 64;
#endif

#ifndef COUNTING_SORT_THRESHOLD_SHORT
constexpr int COUNTING_SORT_THRESHOLD_SHORT = 1750;
#endif

// Missing constants added during refactoring

#ifndef MIN_PARALLEL_MERGE_PARTS_SIZE
constexpr int MIN_PARALLEL_MERGE_PARTS_SIZE = 4096;
#endif

#ifndef MIN_FIRST_RUN_SIZE
constexpr int MIN_FIRST_RUN_SIZE = 16;
#endif

#ifndef MIN_FIRST_RUNS_FACTOR
constexpr int MIN_FIRST_RUNS_FACTOR = 7;
#endif

#ifndef MAX_RUN_CAPACITY
constexpr int MAX_RUN_CAPACITY = 500;
#endif

#ifndef MIN_RUN_COUNT
constexpr int MIN_RUN_COUNT = 5;
#endif

#ifndef MAX_MIXED_INSERTION_SORT_SIZE
constexpr int MAX_MIXED_INSERTION_SORT_SIZE = 48;
#endif

#ifndef MIN_TRY_MERGE_SIZE
constexpr int MIN_TRY_MERGE_SIZE = 64;
#endif

#ifndef DELTA
constexpr int DELTA = 3;
#endif

#ifndef MAX_RECURSION_DEPTH
constexpr int MAX_RECURSION_DEPTH = 64;
#endif

#ifndef MIN_BYTE_COUNTING_SORT_SIZE
constexpr int MIN_BYTE_COUNTING_SORT_SIZE = 64;
#endif

#ifndef MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE
constexpr int MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE = 1750;
#endif

} // namespace dual_pivot

#endif // DPQS_CONSTANTS_HPP
