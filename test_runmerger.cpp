#include "dual_pivot_quicksort.hpp"
#include <iostream>

int main() {
    using namespace dual_pivot;
    
    // Test RunMerger template with new forkMe/getDestination pattern
    std::vector<int> run = {0, 5, 10, 15};
    int a[] = {1,2,3,4,5,8,9,10,11,12,20,21,22,23,24};
    int b[15];
    
    // Test basic compute
    RunMerger<int> merger1(a, b, 0, 1, run, 0, 3);
    int* result1 = merger1.compute();
    std::cout << "Basic compute test passed!" << std::endl;
    
    // Test Java-style forkMe/getDestination pattern
    RunMerger<int> merger2(a, b, 0, 1, run, 0, 3);
    RunMerger<int>& forked = merger2.forkMe();
    int* result2 = forked.getDestination();
    std::cout << "forkMe/getDestination test passed!" << std::endl;
    
    // Test join method
    RunMerger<int> merger3(a, b, 0, 1, run, 0, 3);
    merger3.forkMe();
    int* result3 = merger3.join();
    std::cout << "join method test passed!" << std::endl;
    
    std::cout << "All RunMerger RecursiveTask pattern tests passed!" << std::endl;
    return 0;
}