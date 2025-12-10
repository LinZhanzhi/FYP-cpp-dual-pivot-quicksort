import re
import os

def extract_block(content, start_pattern):
    match = re.search(start_pattern, content, re.DOTALL)
    if not match:
        return None, content

    start_index = match.start()
    brace_start = content.find('{', start_index)
    if brace_start == -1:
        semi_colon = content.find(';', start_index)
        if semi_colon != -1:
            return content[start_index:semi_colon+1], content[:start_index] + content[semi_colon+1:]
        return None, content

    count = 1
    i = brace_start + 1
    while i < len(content) and count > 0:
        if content[i] == '{':
            count += 1
        elif content[i] == '}':
            count -= 1
        i += 1

    while i < len(content) and (content[i] == ';' or content[i].isspace()):
        if content[i] == ';':
            i += 1
            break
        i += 1

    extracted = content[start_index:i]
    remaining = content[:start_index] + content[i:]
    return extracted, remaining

def write_header(path, content, guard):
    header = f'''#ifndef {guard}
#define {guard}

'''
    footer = f'''
#endif // {guard}
'''
    with open(path, 'w') as f:
        f.write(header + content + footer)

with open('include/dpqs/utils.hpp', 'r') as f:
    utils_content = f.read()

with open('include/dual_pivot_quicksort.hpp', 'r') as f:
    main_content = f.read()

array_variant, main_content = extract_block(main_content, r'using\s+ArrayVariant\s*=\s*std::variant')
array_pointer, main_content = extract_block(main_content, r'struct\s+ArrayPointer')
make_array_pointer, main_content = extract_block(main_content, r'template<typename T>\s*ArrayPointer\s+makeArrayPointer')

counted_completer, utils_content = extract_block(utils_content, r'template<typename T>\s*class\s+CountedCompleter')
generic_sorter, utils_content = extract_block(utils_content, r'class\s+GenericSorter')
sorter, utils_content = extract_block(utils_content, r'template<typename T>\s*class\s+Sorter')
generic_merger, utils_content = extract_block(utils_content, r'class\s+GenericMerger')
merger, utils_content = extract_block(utils_content, r'template<typename T>\s*class\s+Merger')

float_bits, main_content = extract_block(main_content, r'inline\s+std::uint32_t\s+floatToRawIntBits')
double_bits, main_content = extract_block(main_content, r'inline\s+std::uint64_t\s+doubleToRawLongBits')

types_content = '''#include <variant>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <cstdint>

namespace dual_pivot {

''' + (array_variant or '') + '\n\n' + (array_pointer or '') + '\n\n' + (make_array_pointer or '') + '\n\n} \n'
write_header('include/dpqs/types.hpp', types_content, 'DPQS_TYPES_HPP')

completer_content = '''#include "dpqs/utils.hpp"
#include <atomic>
#include <mutex>
#include <exception>
#include <thread>
#include <chrono>

namespace dual_pivot {

''' + (counted_completer or '') + '\n\n} \n'
write_header('include/dpqs/parallel/completer.hpp', completer_content, 'DPQS_PARALLEL_COMPLETER_HPP')

merger_content = '''#include "dpqs/parallel/completer.hpp"
#include "dpqs/types.hpp"
#include "dpqs/utils.hpp"

namespace dual_pivot {

''' + (generic_merger or '') + '\n\n' + (merger or '') + '\n\n} \n'
write_header('include/dpqs/parallel/merger.hpp', merger_content, 'DPQS_PARALLEL_MERGER_HPP')

sorter_content = '''#include "dpqs/parallel/completer.hpp"
#include "dpqs/parallel/merger.hpp"
#include "dpqs/types.hpp"
#include "dpqs/utils.hpp"

namespace dual_pivot {

// Forward declarations
template<typename T> class Sorter;
static void sort_int_sequential(Sorter<int>* sorter, int* a, int bits, int low, int high);
static void sort_long_sequential(Sorter<long>* sorter, long* a, int bits, int low, int high);
static void sort_float_sequential(Sorter<float>* sorter, float* a, int bits, int low, int high);
static void sort_double_sequential(Sorter<double>* sorter, double* a, int bits, int low, int high);
template<typename T> void parallelQuickSort(T* a, int depth, int low, int high);

''' + (generic_sorter or '') + '\n\n' + (sorter or '') + '\n\n} \n'
write_header('include/dpqs/parallel/sorter.hpp', sorter_content, 'DPQS_PARALLEL_SORTER_HPP')

utils_content = re.sub(r'template<typename T>\s*class\s+Sorter;\s*', '', utils_content)
utils_content = re.sub(r'template<typename T>\s*class\s+Merger;\s*', '', utils_content)
utils_content = re.sub(r'template<typename T>\s*class\s+RunMerger;\s*', '', utils_content)

with open('include/dpqs/utils.hpp', 'w') as f:
    f.write(utils_content)

includes = '''#include "dpqs/utils.hpp"
#include "dpqs/constants.hpp"
#include "dpqs/types.hpp"
#include "dpqs/parallel/completer.hpp"
#include "dpqs/parallel/merger.hpp"
#include "dpqs/parallel/sorter.hpp"
'''
main_content = main_content.replace('#include "dpqs/utils.hpp"', includes)
main_content = main_content.replace('#include "dpqs/constants.hpp"', '')

with open('include/dual_pivot_quicksort.hpp', 'w') as f:
    f.write(main_content)
