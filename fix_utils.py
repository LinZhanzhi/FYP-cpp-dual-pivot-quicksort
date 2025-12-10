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

with open('include/dpqs/utils.hpp', 'r') as f:
    content = f.read()

# Add PartitionOperation definition
partition_op_def = """
template<typename T>
using PartitionOperation = std::pair<int, int>(*)(T* a, int low, int high, int pivotIndex1, int pivotIndex2);
"""

if 'using PartitionOperation' not in content:
    content = content.replace('template<typename T>\nFORCE_INLINE std::pair<int, int> partition_intrinsic', partition_op_def + '\ntemplate<typename T>\nFORCE_INLINE std::pair<int, int> partition_intrinsic')

# Remove parallel classes
_, content = extract_block(content, r'template<typename T>\s*class\s+CountedCompleter')
_, content = extract_block(content, r'class\s+GenericSorter')
_, content = extract_block(content, r'template<typename T>\s*class\s+Sorter')
_, content = extract_block(content, r'class\s+GenericMerger')
_, content = extract_block(content, r'template<typename T>\s*class\s+Merger')

# Remove forward declarations
content = re.sub(r'template<typename T>\s*class\s+Sorter;\s*', '', content)
content = re.sub(r'template<typename T>\s*class\s+Merger;\s*', '', content)
content = re.sub(r'template<typename T>\s*class\s+RunMerger;\s*', '', content)

# Remove static function declarations that use Sorter
content = re.sub(r'static void sort_int_sequential\(Sorter<int>\*.*?\);', '', content)
content = re.sub(r'static void sort_long_sequential\(Sorter<long>\*.*?\);', '', content)
content = re.sub(r'static void sort_float_sequential\(Sorter<float>\*.*?\);', '', content)
content = re.sub(r'static void sort_double_sequential\(Sorter<double>\*.*?\);', '', content)
content = re.sub(r'static bool tryMergeRuns_int\(Sorter<int>\*.*?\);', '', content)
content = re.sub(r'static bool tryMergeRuns_long\(Sorter<long>\*.*?\);', '', content)
content = re.sub(r'static bool tryMergeRuns_float\(Sorter<float>\*.*?\);', '', content)
content = re.sub(r'static bool tryMergeRuns_double\(Sorter<double>\*.*?\);', '', content)

with open('include/dpqs/utils.hpp', 'w') as f:
    f.write(content)
