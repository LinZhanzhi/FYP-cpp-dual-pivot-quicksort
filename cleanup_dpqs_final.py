import re
def remove_static_specializations(content):
    lines = content.split('\n')
    start_pattern = re.compile(r'.*static.*_(int|long|float|double|byte|char|short).*')
    for line in lines:
        if start_pattern.match(line):
            print(f'Matched: {line.strip()}')
    return content
file_path = 'include/dual_pivot_quicksort.hpp'
with open(file_path, 'r') as f: content = f.read()
remove_static_specializations(content)
