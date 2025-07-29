#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::cout << "Testing basic functionality..." << std::endl;
    
    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
    std::cout << "Original: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    std::sort(data.begin(), data.end());
    
    std::cout << "Sorted: ";
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
    
    std::cout << "Test completed!" << std::endl;
    return 0;
}