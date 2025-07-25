#include "unit_tests.hpp"

int main() {
    unit_tests::TestSuite test_suite;
    test_suite.run_all_tests();
    return 0;
}