#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include "dual_pivot_quicksort.hpp"

int main() {
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 0);
    std::mt19937 g(42);
    std::shuffle(data.begin(), data.end(), g);

    std::cout << "Sorting..." << std::endl;
    dual_pivot::dual_pivot_quicksort(data.begin(), data.end());

    if (std::is_sorted(data.begin(), data.end())) {
        std::cout << "Sorted!" << std::endl;
    } else {
        std::cout << "Not sorted!" << std::endl;
        return 1;
    }
    return 0;
}
