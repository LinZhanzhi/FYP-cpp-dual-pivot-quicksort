#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <numeric>
#include <cmath>
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
void run_test(const std::string& algo, benchmark_data::DataPattern pattern, size_t size, const std::string& output_file, const std::string& type_name, int iterations) {
    auto data = benchmark_data::generate_data<T>(size, pattern);

    // Warmup
    auto warmup_data = data;
    if (algo == "std_sort") {
        std::sort(warmup_data.begin(), warmup_data.end());
    } else if (algo == "std_stable_sort") {
        std::stable_sort(warmup_data.begin(), warmup_data.end());
    } else if (algo == "qsort") {
        std::qsort(warmup_data.data(), warmup_data.size(), sizeof(T), compare<T>);
    } else {
        dual_pivot::sort(warmup_data);
    }

    std::vector<double> durations;
    durations.reserve(iterations);

    for (int i = 0; i < iterations; ++i) {
        // Copy data for each iteration to ensure we are sorting the same unsorted data
        auto test_data = data;

        auto start = std::chrono::high_resolution_clock::now();
        if (algo == "std_sort") {
            std::sort(test_data.begin(), test_data.end());
        } else if (algo == "std_stable_sort") {
            std::stable_sort(test_data.begin(), test_data.end());
        } else if (algo == "qsort") {
            std::qsort(test_data.data(), test_data.size(), sizeof(T), compare<T>);
        } else {
            dual_pivot::sort(test_data);
        }
        auto end = std::chrono::high_resolution_clock::now();
        durations.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }

    // Calculate Representative Value (Minimum Estimator)
    double representative_value = *std::min_element(durations.begin(), durations.end());

    // Output
    std::ofstream out(output_file);
    out << "Algorithm,Type,Pattern,Size,Iteration,Time(ms)" << std::endl;

    // Write all raw samples
    for (size_t i = 0; i < durations.size(); ++i) {
        out << algo << "," << type_name << "," << benchmark_data::pattern_name(pattern) << "," << size << "," << (i + 1) << "," << durations[i] << std::endl;
    }

    // Write Representative Value as the last line
    out << algo << "," << type_name << "," << benchmark_data::pattern_name(pattern) << "," << size << ",Representative," << representative_value << std::endl;

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
        run_test<int>(algo, pattern, size, output, "int", iterations);
    } else if (type == "long") {
        run_test<long>(algo, pattern, size, output, "long", iterations);
    } else if (type == "double") {
        run_test<double>(algo, pattern, size, output, "double", iterations);
    } else {
        std::cerr << "Unknown type: " << type << std::endl;
        return 1;
    }

    return 0;
}
