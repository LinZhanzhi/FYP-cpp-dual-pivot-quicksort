#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <random>
#include <cstdint>
#include <limits>
#include <type_traits>
#include "dual_pivot_quicksort.hpp"

template <typename T>
void save_to_file(const std::string& filename, const std::vector<T>& data) {
    std::ofstream outfile(filename, std::ios::binary);
    if (outfile) {
        outfile.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(T));
        outfile.close();
    }
}

template <typename T>
int run_test(size_t size, const std::string& filename) {
    std::vector<T> data(size);
    std::random_device rd;
    std::mt19937_64 gen(rd()); // Use 64-bit generator for wider range

    if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dist(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max());
        for (auto& val : data) val = dist(gen);
    } else {
        // Handle integer types
        if constexpr (sizeof(T) == 1) {
             std::uniform_int_distribution<int16_t> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
             for (auto& val : data) val = static_cast<T>(dist(gen));
        } else if constexpr (sizeof(T) == 2) {
             std::uniform_int_distribution<int32_t> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
             for (auto& val : data) val = static_cast<T>(dist(gen));
        } else if constexpr (sizeof(T) == 4) {
             std::uniform_int_distribution<int64_t> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
             for (auto& val : data) val = static_cast<T>(dist(gen));
        } else {
             // 64-bit integers
             if constexpr (std::is_signed_v<T>) {
                 std::uniform_int_distribution<int64_t> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
                 for (auto& val : data) val = dist(gen);
             } else {
                 std::uniform_int_distribution<uint64_t> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
                 for (auto& val : data) val = dist(gen);
             }
        }
    }

    // Save input before sorting
    save_to_file(filename, data);

    // Sort
    try {
        dual_pivot::sort(data);
    } catch (const std::exception& e) {
        std::cerr << "Exception during sort: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception during sort" << std::endl;
        return 1;
    }

    // Verify
    if (!std::is_sorted(data.begin(), data.end())) {
        std::cerr << "Sort failed verification!" << std::endl;
        return 1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <type> <size> <output_file>" << std::endl;
        return 1;
    }

    std::string type = argv[1];
    size_t size = std::stoull(argv[2]);
    std::string filename = argv[3];

    if (type == "int8_t") return run_test<int8_t>(size, filename);
    if (type == "uint8_t") return run_test<uint8_t>(size, filename);
    if (type == "int16_t") return run_test<int16_t>(size, filename);
    if (type == "uint16_t") return run_test<uint16_t>(size, filename);
    if (type == "int32_t") return run_test<int32_t>(size, filename);
    if (type == "uint32_t") return run_test<uint32_t>(size, filename);
    if (type == "int64_t") return run_test<int64_t>(size, filename);
    if (type == "uint64_t") return run_test<uint64_t>(size, filename);
    if (type == "float") return run_test<float>(size, filename);
    if (type == "double") return run_test<double>(size, filename);

    std::cerr << "Unknown type: " << type << std::endl;
    return 1;
}