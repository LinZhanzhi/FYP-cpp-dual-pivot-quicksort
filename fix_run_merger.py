import os
import sys
file_path = 'include/dpqs/run_merger.hpp'
if not os.path.exists(file_path): print(f'File not found: {file_path}'); sys.exit(1)
with open(file_path, 'r') as f: content = f.read()
print(f'Read {len(content)} bytes')
start_marker = '// =============================================================================\n// RUN DETECTION AND MERGING IMPLEMENTATION (Phase 3)\n// ============================================================================='
start_idx = content.find(start_marker)
print(f'Start index: {start_idx}')
if start_idx == -1: start_marker = 'RUN DETECTION AND MERGING IMPLEMENTATION (Phase 3)'; start_idx = content.find(start_marker); print(f'Start index (short): {start_idx}')
end_marker = '/**\n * @brief Optimized 5-element sorting network for pivot selection'
end_idx = content.find(end_marker)
print(f'End index: {end_idx}')
if end_idx == -1: end_marker = 'template<typename T>\nFORCE_INLINE void sort5Network'; end_idx = content.find(end_marker); print(f'End index (alt): {end_idx}')
if start_idx == -1 or end_idx == -1: print('Could not find markers'); sys.exit(1)
body = content[start_idx:end_idx]
header = '#ifndef DPQS_RUN_MERGER_HPP\n#define DPQS_RUN_MERGER_HPP\n\n#include <vector>\n#include <algorithm>\n#include "dpqs/constants.hpp"\n#include "dpqs/merge_ops.hpp"\n#include "dpqs/parallel/merger.hpp"\n\nnamespace dual_pivot {\n\n'
footer = '\n\n} // namespace dual_pivot\n\n#endif // DPQS_RUN_MERGER_HPP'
with open(file_path, 'w') as f: f.write(header + body + footer)
print('File updated successfully')
