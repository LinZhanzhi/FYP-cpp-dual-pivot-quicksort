#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <cassert>
#include <iostream>
#include <random>

#include "../include/dual_pivot_quicksort.hpp"
#include "../include/classic_quicksort.hpp"
#include "../include/dual_pivot_optimized.hpp"
#include "data_generator.hpp"

namespace unit_tests {

class TestSuite {
private:
    int tests_run = 0;
    int tests_passed = 0;
    
public:
    void run_all_tests() {
        std::cout << "Running Comprehensive Unit Tests\n";
        std::cout << "================================\n\n";
        
        // Basic functionality tests
        test_empty_array();
        test_single_element();
        test_two_elements();
        test_sorted_array();
        test_reverse_sorted_array();
        test_all_same_elements();
        test_basic_random_array();
        
        // Edge case tests
        test_large_arrays();
        test_many_duplicates();
        test_nearly_sorted();
        
        // Type tests
        test_different_types();
        test_custom_comparator();
        
        // Algorithm comparison tests
        test_algorithm_equivalence();
        
        // Performance characteristic tests
        test_performance_patterns();
        
        print_summary();
    }
    
private:
    void assert_test(bool condition, const std::string& test_name) {
        tests_run++;
        if (condition) {
            tests_passed++;
            std::cout << "✓ " << test_name << "\n";
        } else {
            std::cout << "✗ " << test_name << " FAILED\n";
        }
    }
    
    template<typename Container>
    bool is_sorted_correctly(const Container& original, const Container& sorted) {
        // Check if sorted
        if (!std::is_sorted(sorted.begin(), sorted.end())) {
            return false;
        }
        
        // Check if it's a permutation
        Container orig_copy = original;
        std::sort(orig_copy.begin(), orig_copy.end());
        return orig_copy == sorted;
    }
    
    void test_empty_array() {
        std::vector<int> empty;
        dual_pivot::dual_pivot_quicksort(empty.begin(), empty.end());
        assert_test(empty.empty(), "Empty array test");
    }
    
    void test_single_element() {
        std::vector<int> single = {42};
        dual_pivot::dual_pivot_quicksort(single.begin(), single.end());
        assert_test(single == std::vector<int>{42}, "Single element test");
    }
    
    void test_two_elements() {
        std::vector<int> two_asc = {1, 2};
        std::vector<int> two_desc = {2, 1};
        
        dual_pivot::dual_pivot_quicksort(two_asc.begin(), two_asc.end());
        dual_pivot::dual_pivot_quicksort(two_desc.begin(), two_desc.end());
        
        bool passed = (two_asc == std::vector<int>{1, 2}) && 
                     (two_desc == std::vector<int>{1, 2});
        assert_test(passed, "Two elements test");
    }
    
    void test_sorted_array() {
        std::vector<int> sorted = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        auto original = sorted;
        dual_pivot::dual_pivot_quicksort(sorted.begin(), sorted.end());
        assert_test(sorted == original, "Already sorted array test");
    }
    
    void test_reverse_sorted_array() {
        std::vector<int> reverse = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        std::vector<int> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        dual_pivot::dual_pivot_quicksort(reverse.begin(), reverse.end());
        assert_test(reverse == expected, "Reverse sorted array test");
    }
    
    void test_all_same_elements() {
        std::vector<int> same = {5, 5, 5, 5, 5, 5, 5};
        auto original = same;
        dual_pivot::dual_pivot_quicksort(same.begin(), same.end());
        assert_test(same == original, "All same elements test");
    }
    
    void test_basic_random_array() {
        std::vector<int> random = {64, 34, 25, 12, 22, 11, 90, 5, 77, 30};
        auto original = random;
        dual_pivot::dual_pivot_quicksort(random.begin(), random.end());
        assert_test(is_sorted_correctly(original, random), "Basic random array test");
    }
    
    void test_large_arrays() {
        auto large_array = benchmark_data::generate_data<int>(10000, benchmark_data::DataPattern::RANDOM);
        auto original = large_array;
        dual_pivot::dual_pivot_quicksort(large_array.begin(), large_array.end());
        assert_test(is_sorted_correctly(original, large_array), "Large array (10,000 elements) test");
    }
    
    void test_many_duplicates() {
        auto duplicates = benchmark_data::generate_data<int>(1000, benchmark_data::DataPattern::MANY_DUPLICATES_10);
        auto original = duplicates;
        dual_pivot::dual_pivot_quicksort(duplicates.begin(), duplicates.end());
        assert_test(is_sorted_correctly(original, duplicates), "Many duplicates test");
    }
    
    void test_nearly_sorted() {
        auto nearly_sorted = benchmark_data::generate_data<int>(1000, benchmark_data::DataPattern::NEARLY_SORTED);
        auto original = nearly_sorted;
        dual_pivot::dual_pivot_quicksort(nearly_sorted.begin(), nearly_sorted.end());
        assert_test(is_sorted_correctly(original, nearly_sorted), "Nearly sorted test");
    }
    
    void test_different_types() {
        // Test with doubles
        std::vector<double> doubles = {3.14, 2.71, 1.41, 1.73, 0.57, 2.23};
        auto doubles_orig = doubles;
        dual_pivot::dual_pivot_quicksort(doubles.begin(), doubles.end());
        bool doubles_ok = is_sorted_correctly(doubles_orig, doubles);
        
        // Test with strings
        std::vector<std::string> strings = {"zebra", "apple", "banana", "cherry", "date"};
        auto strings_orig = strings;
        dual_pivot::dual_pivot_quicksort(strings.begin(), strings.end());
        bool strings_ok = is_sorted_correctly(strings_orig, strings);
        
        assert_test(doubles_ok && strings_ok, "Different types test");
    }
    
    void test_custom_comparator() {
        std::vector<int> data = {1, 5, 3, 9, 2, 8, 4, 7, 6};
        
        // Sort in descending order
        dual_pivot::dual_pivot_quicksort(data.begin(), data.end(), std::greater<int>());
        
        std::vector<int> expected = {9, 8, 7, 6, 5, 4, 3, 2, 1};
        assert_test(data == expected, "Custom comparator test");
    }
    
    void test_algorithm_equivalence() {
        const int test_size = 1000;
        bool all_equivalent = true;
        
        for (auto pattern : benchmark_data::all_patterns) {
            auto data = benchmark_data::generate_data<int>(test_size, pattern);
            
            auto data1 = data;  // for std::sort
            auto data2 = data;  // for dual-pivot
            auto data3 = data;  // for classic quicksort
            auto data4 = data;  // for optimized dual-pivot
            
            std::sort(data1.begin(), data1.end());
            dual_pivot::dual_pivot_quicksort(data2.begin(), data2.end());
            classic_quicksort::quicksort(data3.begin(), data3.end());
            dual_pivot_optimized::dual_pivot_introsort(data4.begin(), data4.end());
            
            if (!(data1 == data2 && data2 == data3 && data3 == data4)) {
                all_equivalent = false;
                break;
            }
        }
        
        assert_test(all_equivalent, "Algorithm equivalence test");
    }
    
    void test_performance_patterns() {
        // Test that dual-pivot can handle problematic patterns for classic quicksort
        
        // All same elements (worst case for some quicksort variants)
        std::vector<int> all_same(1000, 42);
        auto original = all_same;
        dual_pivot::dual_pivot_quicksort(all_same.begin(), all_same.end());
        bool same_ok = (all_same == original);
        
        // Organ pipe pattern
        auto organ_pipe = benchmark_data::generate_data<int>(1000, benchmark_data::DataPattern::ORGAN_PIPE);
        auto organ_original = organ_pipe;
        dual_pivot::dual_pivot_quicksort(organ_pipe.begin(), organ_pipe.end());
        bool organ_ok = is_sorted_correctly(organ_original, organ_pipe);
        
        assert_test(same_ok && organ_ok, "Performance patterns test");
    }
    
    void print_summary() {
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "Test Summary: " << tests_passed << "/" << tests_run << " tests passed\n";
        
        if (tests_passed == tests_run) {
            std::cout << "🎉 All tests PASSED! Implementation appears correct.\n";
        } else {
            std::cout << "❌ Some tests FAILED. Review implementation.\n";
        }
        std::cout << std::string(50, '=') << "\n\n";
    }
};

} // namespace unit_tests