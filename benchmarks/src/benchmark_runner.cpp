#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <numeric>
#include <cmath>
#include <cstdint>
#include "dual_pivot_quicksort.hpp"
#include "timer.hpp"
#include "data_generator.hpp"

template <typename T>
int compare(const void* a, const void* b) {
    T arg1 = *static_cast<const T*>(a);
    T arg2 = *static_cast<const T*>(b);
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

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

template <typename T>
void run_test(const std::string& algo, benchmark_data::DataPattern pattern, size_t size,
              const std::string& output_file, const std::string& type_name,
              int iterations, int threads, int num_seeds, unsigned base_seed) {
    std::vector<double> all_durations;       // flat list of every timing
    std::vector<unsigned> all_seeds;         // seed paired with each timing
    std::vector<double> per_seed_medians;    // one median per seed
    all_durations.reserve(static_cast<size_t>(num_seeds) * iterations);
    all_seeds.reserve(static_cast<size_t>(num_seeds) * iterations);
    per_seed_medians.reserve(num_seeds);

    bool correctness_checked = false;

    for (int s = 0; s < num_seeds; ++s) {
        unsigned seed = base_seed + static_cast<unsigned>(s);
        auto data = benchmark_data::generate_data<T>(size, pattern, seed);

        // Warmup: 3 iterations to warm caches / thread pools
        for (int w = 0; w < 3; ++w) {
            auto warmup_data = data;
            if (algo == "std_sort") {
                std::sort(warmup_data.begin(), warmup_data.end());
            } else if (algo == "std_stable_sort") {
                std::stable_sort(warmup_data.begin(), warmup_data.end());
            } else if (algo == "qsort") {
                std::qsort(warmup_data.data(), warmup_data.size(), sizeof(T), compare<T>);
            } else if (algo.find("dual_pivot_parallel") != std::string::npos) {
                dual_pivot::sort(warmup_data, threads);
            } else if (algo == "dual_pivot_sequential") {
                dual_pivot::sort(warmup_data, 1);
            } else {
                dual_pivot::sort(warmup_data);
            }
            if (!correctness_checked) {
                if (!std::is_sorted(warmup_data.begin(), warmup_data.end())) {
                    std::cerr << "Error: Algorithm " << algo << " failed to sort the array correctly." << std::endl;
                    std::exit(1);
                }
                correctness_checked = true;
            }
        }

        std::vector<double> seed_times;
        seed_times.reserve(iterations);
        for (int i = 0; i < iterations; ++i) {
            auto test_data = data;
            auto start = std::chrono::high_resolution_clock::now();
            if (algo == "std_sort") {
                std::sort(test_data.begin(), test_data.end());
            } else if (algo == "std_stable_sort") {
                std::stable_sort(test_data.begin(), test_data.end());
            } else if (algo == "qsort") {
                std::qsort(test_data.data(), test_data.size(), sizeof(T), compare<T>);
            } else if (algo.find("dual_pivot_parallel") != std::string::npos) {
                dual_pivot::sort(test_data, threads);
            } else if (algo == "dual_pivot_sequential") {
                dual_pivot::sort(test_data, 1);
            } else {
                dual_pivot::sort(test_data);
            }
            auto end = std::chrono::high_resolution_clock::now();
            double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
            seed_times.push_back(elapsed_ms);
            all_durations.push_back(elapsed_ms);
            all_seeds.push_back(seed);

            // Lightweight sortedness sanity check
            if (test_data.size() > 1 && test_data[0] > test_data[1]) {
                std::cerr << "Warning: Iteration " << i << " (seed " << seed
                          << ") potentially not sorted (first elements check)" << std::endl;
            }
            volatile auto sink = test_data.front();
            (void)sink;

            std::this_thread::yield();
        }

        // Per-seed median (used for median-of-medians representative)
        std::vector<double> sorted_times = seed_times;
        std::sort(sorted_times.begin(), sorted_times.end());
        double median_seed = sorted_times.empty()
            ? 0.0
            : sorted_times[sorted_times.size() / 2];
        per_seed_medians.push_back(median_seed);
    }

    // Representative:
    //   - num_seeds > 1  -> median of per-seed medians (industry-standard for randomized input)
    //   - num_seeds == 1 -> minimum (backward-compatible behavior)
    double representative_value = 0.0;
    if (num_seeds > 1) {
        std::vector<double> sorted_medians = per_seed_medians;
        std::sort(sorted_medians.begin(), sorted_medians.end());
        representative_value = sorted_medians[sorted_medians.size() / 2];
    } else if (!all_durations.empty()) {
        representative_value = *std::min_element(all_durations.begin(), all_durations.end());
    }

    // Output
    std::ofstream out(output_file);
    out << "Algorithm,Type,Pattern,Size,Iteration,Time(ms),Seed" << std::endl;
    for (size_t i = 0; i < all_durations.size(); ++i) {
        out << algo << "," << type_name << "," << benchmark_data::pattern_name(pattern)
            << "," << size << "," << (i + 1) << ","
            << all_durations[i] << "," << all_seeds[i] << std::endl;
    }
    out << algo << "," << type_name << "," << benchmark_data::pattern_name(pattern)
        << "," << size << ",Representative," << representative_value << "," << base_seed << std::endl;

    out.close();
}

int main(int argc, char* argv[]) {
    auto args = parse_args(argc, argv);
    if (args.find("algorithm") == args.end() || args.find("type") == args.end() || args.find("pattern") == args.end() || args.find("size") == args.end() || args.find("output") == args.end()) {
        std::cerr << "Usage error" << std::endl;
        return 1;
    }
    std::string algo = args["algorithm"];
    std::string type = args["type"];
    std::string pattern_str = args["pattern"];
    size_t size = std::stoull(args["size"]);
    std::string output = args["output"];
    int iterations = 1;
    if (args.find("iterations") != args.end()) {
        iterations = std::stoi(args["iterations"]);
    }

    int threads = std::thread::hardware_concurrency();
    if (args.find("threads") != args.end()) {
        threads = std::stoi(args["threads"]);
    }

    // Multi-seed support for statistically robust random-input benchmarks.
    // seeds=1 (default) preserves legacy single-seed behavior.
    int num_seeds = 1;
    if (args.find("seeds") != args.end()) {
        num_seeds = std::stoi(args["seeds"]);
        if (num_seeds < 1) num_seeds = 1;
    }
    unsigned base_seed = 42;
    if (args.find("base-seed") != args.end()) {
        base_seed = static_cast<unsigned>(std::stoul(args["base-seed"]));
    }

    benchmark_data::DataPattern pattern;
    if (pattern_str == "RANDOM") pattern = benchmark_data::DataPattern::RANDOM;
    else if (pattern_str == "NEARLY_SORTED") pattern = benchmark_data::DataPattern::NEARLY_SORTED;
    else if (pattern_str == "REVERSE_SORTED") pattern = benchmark_data::DataPattern::REVERSE_SORTED;
    else if (pattern_str == "MANY_DUPLICATES_10") pattern = benchmark_data::DataPattern::MANY_DUPLICATES_10;
    else if (pattern_str == "MANY_DUPLICATES_50") pattern = benchmark_data::DataPattern::MANY_DUPLICATES_50;
    else if (pattern_str == "MANY_DUPLICATES_90") pattern = benchmark_data::DataPattern::MANY_DUPLICATES_90;
    else if (pattern_str == "ORGAN_PIPE") pattern = benchmark_data::DataPattern::ORGAN_PIPE;
    else if (pattern_str == "SAWTOOTH") pattern = benchmark_data::DataPattern::SAWTOOTH;
    else {
        std::cerr << "Unknown pattern: " << pattern_str << std::endl;
        return 1;
    }
    if (type == "int") {
        run_test<int>(algo, pattern, size, output, "int", iterations, threads, num_seeds, base_seed);
    } else if (type == "int8_t") {
        run_test<std::int8_t>(algo, pattern, size, output, "int8_t", iterations, threads, num_seeds, base_seed);
    } else if (type == "int16_t") {
        run_test<std::int16_t>(algo, pattern, size, output, "int16_t", iterations, threads, num_seeds, base_seed);
    } else if (type == "long") {
        run_test<long>(algo, pattern, size, output, "long", iterations, threads, num_seeds, base_seed);
    } else if (type == "double") {
        run_test<double>(algo, pattern, size, output, "double", iterations, threads, num_seeds, base_seed);
    } else {
        std::cerr << "Unknown type: " << type << std::endl;
        return 1;
    }

    return 0;
}
