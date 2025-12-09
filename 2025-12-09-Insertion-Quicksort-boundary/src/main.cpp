#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <string>
#include <functional>
#include "sort.hpp"

enum class Distribution {
    RANDOM,
    NEARLY_SORTED,
    REVERSE_SORTED,
    FEW_UNIQUE
};

std::vector<int> generate_data(int size, Distribution dist) {
    std::vector<int> data(size);
    std::mt19937 rng(42); // Fixed seed for reproducibility

    switch (dist) {
        case Distribution::RANDOM: {
            std::uniform_int_distribution<int> uni(0, 1000000);
            for (int& x : data) x = uni(rng);
            break;
        }
        case Distribution::NEARLY_SORTED: {
            for (int i = 0; i < size; ++i) data[i] = i;
            std::uniform_int_distribution<int> idx_dist(0, size - 1);
            // Swap about 1% of elements
            for (int i = 0; i < size / 100; ++i) {
                std::swap(data[idx_dist(rng)], data[idx_dist(rng)]);
            }
            break;
        }
        case Distribution::REVERSE_SORTED: {
            for (int i = 0; i < size; ++i) data[i] = size - i;
            break;
        }
        case Distribution::FEW_UNIQUE: {
            std::uniform_int_distribution<int> few(0, 10);
            for (int& x : data) x = few(rng);
            break;
        }
    }
    return data;
}

Distribution parse_dist(const std::string& s) {
    if (s == "random") return Distribution::RANDOM;
    if (s == "sorted") return Distribution::NEARLY_SORTED; // "nearly sorted"
    if (s == "reverse") return Distribution::REVERSE_SORTED;
    if (s == "few") return Distribution::FEW_UNIQUE;
    return Distribution::RANDOM;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <size> <threshold> <distribution>" << std::endl;
        return 1;
    }

    int size = std::stoi(argv[1]);
    int threshold = std::stoi(argv[2]);
    std::string dist_str = argv[3];
    Distribution dist = parse_dist(dist_str);

    // Warmup
    {
        auto warmup_data = generate_data(1000, Distribution::RANDOM);
        hybrid_quicksort(warmup_data.begin(), warmup_data.end(), threshold);
    }

    // Generate Data
    auto data = generate_data(size, dist);

    // Measure Time
    auto start = std::chrono::high_resolution_clock::now();
    hybrid_quicksort(data.begin(), data.end(), threshold);
    auto end = std::chrono::high_resolution_clock::now();

    // Verify sorted
    if (!std::is_sorted(data.begin(), data.end())) {
        std::cerr << "Error: Array not sorted!" << std::endl;
        return 1;
    }

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << duration << std::endl;

    return 0;
}
