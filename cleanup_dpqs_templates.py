import os
file_path = 'include/dual_pivot_quicksort.hpp'
with open(file_path, 'r') as f: content = f.read()
start_marker_1 = '/**\n * @brief Optimized dual-pivot partitioning with cache-aware memory access'
end_marker_2 = '/**\n * @brief Optimized 5-element sorting network for pivot selection'
start_idx_1 = content.find(start_marker_1)
end_idx_2 = content.find(end_marker_2)
if end_idx_2 == -1: end_marker_2 = 'template<typename T>\nFORCE_INLINE void sort5Network'; end_idx_2 = content.find(end_marker_2)
print(f'Start: {start_idx_1}, End: {end_idx_2}')
if start_idx_1 != -1 and end_idx_2 != -1:
    new_content = content[:start_idx_1] + content[end_idx_2:]
    include_marker = '#include "dpqs/sequential_sorters.hpp"'
    include_idx = new_content.find(include_marker)
    if include_idx != -1:
        insert_pos = include_idx + len(include_marker)
        new_content = new_content[:insert_pos] + '\n#include "dpqs/partition.hpp"\n#include "dpqs/run_merger.hpp"' + new_content[insert_pos:]
    with open(file_path, 'w') as f: f.write(new_content)
    print('Updated dual_pivot_quicksort.hpp')
else:
    print(f'Could not find range to remove: {start_idx_1}, {end_idx_2}')
