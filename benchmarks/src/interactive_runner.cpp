#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iomanip>
#include <fstream>
#include "dual_pivot_quicksort.hpp"
#include "data_generator.hpp"

// Simple JSON printer helper
void print_json_value(const std::string& key, const std::string& value, bool last = false) {
    std::cout << "\"" << key << "\": \"" << value << "\"" << (last ? "" : ",") << std::endl;
}

void print_json_value(const std::string& key, double value, bool last = false) {
    std::cout << "\"" << key << "\": " << value << (last ? "" : ",") << std::endl;
}

template <typename T>
void print_array(const std::vector<T>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i];
        if (i < arr.size() - 1) std::cout << ", ";
    }
    std::cout << "]";
}

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
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.length() > 2 && arg.substr(0, 2) == "--") {
            std::string key = arg.substr(2);
            if (i + 1 < argc && std::string(argv[i+1]).substr(0, 2) != "--") {
                args[key] = argv[i+1];
                i++;
            } else {
                args[key] = "";
            }
        }
    }
    return args;
}

template <typename T>
double run_algo(const std::string& algo, const std::vector<T>& original_data, int iterations) {
    // Warmup run (discarded)
    {
        std::vector<T> warmup_data = original_data;
        if (algo == "std_sort") {
            std::sort(warmup_data.begin(), warmup_data.end());
        } else if (algo == "std_stable_sort") {
            std::stable_sort(warmup_data.begin(), warmup_data.end());
        } else if (algo == "qsort") {
            std::qsort(warmup_data.data(), warmup_data.size(), sizeof(T), compare<T>);
        } else if (algo == "dual_pivot") {
            dual_pivot::sort(warmup_data);
        }
    }

    std::vector<double> durations;
    durations.reserve(iterations);

    for (int i = 0; i < iterations; ++i) {
        std::vector<T> test_data = original_data;
        auto start = std::chrono::high_resolution_clock::now();

        if (algo == "std_sort") {
            std::sort(test_data.begin(), test_data.end());
        } else if (algo == "std_stable_sort") {
            std::stable_sort(test_data.begin(), test_data.end());
        } else if (algo == "qsort") {
            std::qsort(test_data.data(), test_data.size(), sizeof(T), compare<T>);
        } else if (algo == "dual_pivot") {
            dual_pivot::sort(test_data);
        }

        auto end = std::chrono::high_resolution_clock::now();
        durations.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }

    // Calculate Statistics (Mean)
    double sum = std::accumulate(durations.begin(), durations.end(), 0.0);
    double mean = sum / durations.size();

    // StdDev
    double sq_sum = 0.0;
    for (double d : durations) {
        sq_sum += (d - mean) * (d - mean);
    }
    double stdev = (durations.size() > 1) ? std::sqrt(sq_sum / (durations.size() - 1)) : 0.0;

    // Filter Outliers
    std::vector<double> filtered;
    double lower_bound = mean - 2 * stdev;
    double upper_bound = mean + 2 * stdev;

    for (double d : durations) {
        if (d >= lower_bound && d <= upper_bound) {
            filtered.push_back(d);
        }
    }

    if (!filtered.empty()) {
        return std::accumulate(filtered.begin(), filtered.end(), 0.0) / filtered.size();
    }
    return mean;
}

template <typename T>
void run_interactive(size_t size, benchmark_data::DataPattern pattern, bool only_generate, const std::vector<T>& input_data = {}) {
    std::vector<T> data;
    if (only_generate) {
        data = benchmark_data::generate_data<T>(size, pattern);
        std::cout << "{" << std::endl;
        print_json_value("size", std::to_string(size));
        std::cout << "\"unsorted_array\": ";
        print_array(data);
        std::cout << std::endl;
        std::cout << "}" << std::endl;
        return;
    } else {
        if (input_data.empty()) {
             // Fallback if no input data provided (should not happen in new flow but good for safety)
             data = benchmark_data::generate_data<T>(size, pattern);
        } else {
            data = input_data;
        }
    }

    // Generate sorted version for display (using std::sort as reference)
    std::vector<T> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());

    std::vector<std::string> algos = {"std_sort", "dual_pivot", "std_stable_sort", "qsort"};

    std::cout << "{" << std::endl;
    print_json_value("size", std::to_string(size));

    std::cout << "\"results\": [" << std::endl;
    for (size_t i = 0; i < algos.size(); ++i) {
        double runtime = run_algo(algos[i], data, 30);
        std::cout << "  { \"algorithm\": \"" << algos[i] << "\", \"runtime\": " << runtime << " }" << (i < algos.size() - 1 ? "," : "") << std::endl;
    }
    std::cout << "]," << std::endl;

    std::cout << "\"sorted_array\": ";
    print_array(sorted_data);
    std::cout << std::endl;
    std::cout << "}" << std::endl;
}

// Helper to parse array from string (simple comma separated)
template <typename T>
std::vector<T> parse_array(const std::string& str) {
    std::vector<T> result;
    std::stringstream ss(str);
    T val;
    while (ss >> val) {
        result.push_back(val);
        if (ss.peek() == ',') ss.ignore();
    }
    return result;
}

// Helper to parse array from file
template <typename T>
std::vector<T> parse_array_from_file(const std::string& filepath) {
    std::vector<T> result;
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        exit(1);
    }

    T val;
    while (infile >> val) {
        result.push_back(val);
        char c;
        // Peek to skip commas if present
        while (infile.peek() == ',' || std::isspace(infile.peek())) {
            infile.get(c);
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    auto args = parse_args(argc, argv);

    bool only_generate = args.count("generate");
    bool run_sort = args.count("sort");

    if (only_generate) {
        if (args.find("size") == args.end() || args.find("pattern") == args.end()) {
            std::cerr << "Usage: --generate --size <n> --pattern <p> [--type <int|double>]" << std::endl;
            return 1;
        }
    } else if (run_sort) {
         if (args.find("data") == args.end() && args.find("data-file") == args.end()) {
            std::cerr << "Usage: --sort [--data <values> | --data-file <path>] [--type <int|double>]" << std::endl;
            return 1;
        }
    } else {
         // Legacy mode or invalid
         if (args.find("size") == args.end() || args.find("pattern") == args.end()) {
            std::cerr << "Usage: --size <n> --pattern <p> [--type <int|double>]" << std::endl;
            return 1;
        }
    }

    std::string type = args.count("type") ? args["type"] : "int";

    if (only_generate) {
        size_t size = std::stoul(args["size"]);
        std::string pattern_str = args["pattern"];
        benchmark_data::DataPattern pattern;
        if (pattern_str == "RANDOM") pattern = benchmark_data::DataPattern::RANDOM;
        else if (pattern_str == "NEARLY_SORTED") pattern = benchmark_data::DataPattern::NEARLY_SORTED;
        else if (pattern_str == "REVERSE_SORTED") pattern = benchmark_data::DataPattern::REVERSE_SORTED;
        else if (pattern_str == "MANY_DUPLICATES_10") pattern = benchmark_data::DataPattern::MANY_DUPLICATES_10;
        else if (pattern_str == "MANY_DUPLICATES_50") pattern = benchmark_data::DataPattern::MANY_DUPLICATES_50;
        else if (pattern_str == "MANY_DUPLICATES_90") pattern = benchmark_data::DataPattern::MANY_DUPLICATES_90;
        else if (pattern_str == "ORGAN_PIPE") pattern = benchmark_data::DataPattern::ORGAN_PIPE;
        else if (pattern_str == "SAWTOOTH") pattern = benchmark_data::DataPattern::SAWTOOTH;
        else pattern = benchmark_data::DataPattern::RANDOM;

        if (type == "double") run_interactive<double>(size, pattern, true);
        else run_interactive<int>(size, pattern, true);
    } else if (run_sort) {
        if (args.count("data-file")) {
            std::string filepath = args["data-file"];
            if (type == "double") {
                auto data = parse_array_from_file<double>(filepath);
                run_interactive<double>(data.size(), benchmark_data::DataPattern::RANDOM, false, data);
            } else {
                auto data = parse_array_from_file<int>(filepath);
                run_interactive<int>(data.size(), benchmark_data::DataPattern::RANDOM, false, data);
            }
        } else {
            std::string data_str = args["data"];
            // Remove brackets if present
            data_str.erase(std::remove(data_str.begin(), data_str.end(), '['), data_str.end());
            data_str.erase(std::remove(data_str.begin(), data_str.end(), ']'), data_str.end());

            if (type == "double") {
                auto data = parse_array<double>(data_str);
                run_interactive<double>(data.size(), benchmark_data::DataPattern::RANDOM, false, data);
            } else {
                auto data = parse_array<int>(data_str);
                run_interactive<int>(data.size(), benchmark_data::DataPattern::RANDOM, false, data);
            }
        }
    } else {
        // Legacy path (keep for compatibility if needed, or just reuse generate logic but run both)
        // For now, let's just support the split workflow as requested.
        // But to be safe, let's keep the old logic if neither flag is present?
        // Actually, the user asked to split it.
        size_t size = std::stoul(args["size"]);
        std::string pattern_str = args["pattern"];
        benchmark_data::DataPattern pattern;
        if (pattern_str == "RANDOM") pattern = benchmark_data::DataPattern::RANDOM;
        // ... (rest of pattern parsing)
        else pattern = benchmark_data::DataPattern::RANDOM;

        if (type == "double") run_interactive<double>(size, pattern, false); // This will generate and run
        else run_interactive<int>(size, pattern, false);
    }

    return 0;
}
