#ifndef DPQS_CONSTANTS_HPP
#define DPQS_CONSTANTS_HPP

namespace dual_pivot {

// Constants
constexpr int MAX_INSERTION_SORT_SIZE = 44;
constexpr int MIN_PARALLEL_SORT_SIZE = 4096;
constexpr int MAX_RUN_COUNT = 67;
constexpr int MAX_RUN_LENGTH = 33;
constexpr int QUICKSORT_THRESHOLD = 286;
constexpr int COUNTING_SORT_THRESHOLD_BYTE = 29;
constexpr int COUNTING_SORT_THRESHOLD_SHORT = 3200;

// Missing constants added during refactoring
constexpr int MIN_PARALLEL_MERGE_PARTS_SIZE = 4096;
constexpr int MIN_FIRST_RUN_SIZE = 16;
constexpr int MIN_FIRST_RUNS_FACTOR = 7;
constexpr int MAX_RUN_CAPACITY = 5120;
constexpr int MIN_RUN_COUNT = 4;
constexpr int MAX_MIXED_INSERTION_SORT_SIZE = 65;
constexpr int MIN_TRY_MERGE_SIZE = 4096;
constexpr int DELTA = 6;
constexpr int MAX_RECURSION_DEPTH = 384;
constexpr int MIN_BYTE_COUNTING_SORT_SIZE = 64;
constexpr int MIN_SHORT_OR_CHAR_COUNTING_SORT_SIZE = 1750;

} // namespace dual_pivot

#endif // DPQS_CONSTANTS_HPP
