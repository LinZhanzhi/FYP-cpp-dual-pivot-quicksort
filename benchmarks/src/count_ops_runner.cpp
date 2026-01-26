#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <numeric>
#include <algorithm>
#include "dual_pivot_quicksort.hpp"
#include "data_generator.hpp"
#include "instrumented.hpp"

// Argument parsing
std::map<std::string, std::string> parse_args(int argc, char* argv[]) {
    std::map<std::string, std::string> args;
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 < argc) {
            std::string key = argv[i];
            if (key.substr(0, 2) == "--") {
                key = key.substr(2);
                args[key] = argv[i + 1];
            }
        }
    }
    return args;
}

// Convert string pattern to enum
benchmark_data::DataPattern get_pattern(const std::string& pattern_str) {
    if (pattern_str == "RANDOM") return benchmark_data::DataPattern::RANDOM;
    if (pattern_str == "NEARLY_SORTED") return benchmark_data::DataPattern::NEARLY_SORTED;
    if (pattern_str == "REVERSE_SORTED") return benchmark_data::DataPattern::REVERSE_SORTED;
    if (pattern_str == "MANY_DUPLICATES_10") return benchmark_data::DataPattern::MANY_DUPLICATES_10;
    if (pattern_str == "MANY_DUPLICATES_50") return benchmark_data::DataPattern::MANY_DUPLICATES_50;
    if (pattern_str == "MANY_DUPLICATES_90") return benchmark_data::DataPattern::MANY_DUPLICATES_90;
    if (pattern_str == "ORGAN_PIPE") return benchmark_data::DataPattern::ORGAN_PIPE;
    if (pattern_str == "SAWTOOTH") return benchmark_data::DataPattern::SAWTOOTH;
    return benchmark_data::DataPattern::RANDOM;
}

int main(int argc, char* argv[]) {
    auto args = parse_args(argc, argv);

    if (args.find("size") == args.end() || args.find("pattern") == args.end()) {
        std::cerr << "Usage: --size <n> --pattern <p> [--type <t>]" << std::endl;
        return 1;
    }

    size_t size = std::stoull(args["size"]);
    std::string pattern_str = args["pattern"];
    std::string algo = args["algo"];

    // We only support dual_pivot_sequential logic here for counting.
    // If user passed std_sort, we technically could support it if we wrap it.
    // But for now let's focus on checking *our* algorithm's counts.

    // Generate raw data
    // Use int as standard proxy for operations count
    auto raw_data = benchmark_data::generate_data<int>(size, get_pattern(pattern_str));

    // Convert to Instrumented
    std::vector<Instrumented<int>> data;
    data.reserve(size);
    for (int x : raw_data) {
        data.emplace_back(x);
    }

    // Reset counters
    Instrumented<int>::reset();

    // Determine parallelism for dual_pivot
    int parallelism = 0;
    if (algo.find("dual_pivot_parallel_") == 0) {
        try {
            parallelism = std::stoi(algo.substr(20));
        } catch (...) {
            parallelism = 0;
        }
    }

    // Run Sort

    if (algo == "std_sort") {
        std::sort(data.begin(), data.end());
    } else if (algo == "std_stable_sort") {
        std::stable_sort(data.begin(), data.end());
    } else {
        // Default to dual pivot (sequential or parallel based on algo string)
        dual_pivot::sort(data.data(), parallelism, 0, data.size(), std::less<Instrumented<int>>());
    }

    // Output results: comparisons,swaps,assignments
    std::cout << Instrumented<int>::comparisons << "," << Instrumented<int>::swaps << "," << Instrumented<int>::assignments << std::flush;

    return 0;
}
