import re
def remove_functions(content):
    prefixes = ["mixedInsertionSort", "partitionDualPivot", "partitionSinglePivot", "sort", "tryMergeRuns", "mergeRuns", "countingSort"]
    types = ["int", "long", "float", "double", "byte", "char", "short"]
    p1 = r"static\s+.*?\s+("
    p2 = "|".join(prefixes) + r")_("
    p3 = "|".join(types) + r")(_\w+)?\s*\([^)]*\)\s*\{"
    pattern = re.compile(p1 + p2 + p3)
    lines = content.split("\n")
    keep_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        match = pattern.search(line)
        if match:
            print(f"Removing: {line.strip()}")
            brace_count = 0
            brace_count += line.count("{")
            brace_count -= line.count("}")
            j = i + 1
            while j < len(lines) and brace_count > 0:
                brace_count += lines[j].count("{")
                brace_count -= lines[j].count("}")
                j += 1
            i = j
        else:
            keep_lines.append(line)
            i += 1
    return "\n".join(keep_lines)

def remove_templates(content):
    patterns = [
        r"FORCE_INLINE void sort5Network",
        r"FORCE_INLINE void sort_intrinsic",
        r"FORCE_INLINE std::pair<int, int> partition_intrinsic"
    ]
    lines = content.split("\n")
    keep_lines = []
    i = 0
    while i < len(lines):
        line = lines[i]
        matched = False
        for p in patterns:
            if re.search(p, line):
                matched = True
                break
        if matched:
            print(f"Removing template: {line.strip()}")
            # Check if previous line was template declaration
            if len(keep_lines) > 0 and "template" in keep_lines[-1]:
                keep_lines.pop()
            if len(keep_lines) > 0 and "//" in keep_lines[-1]:
                keep_lines.pop()
            brace_count = 0
            brace_count += line.count("{")
            brace_count -= line.count("}")
            j = i + 1
            while j < len(lines) and brace_count > 0:
                brace_count += lines[j].count("{")
                brace_count -= lines[j].count("}")
                j += 1
            i = j
        else:
            keep_lines.append(line)
            i += 1
    return "\n".join(keep_lines)

with open("/home/lzz725/FYP/include/dual_pivot_quicksort.hpp", "r") as f:
    content = f.read()
content = remove_functions(content)
content = remove_templates(content)
with open("/home/lzz725/FYP/include/dual_pivot_quicksort.hpp", "w") as f:
    f.write(content)
