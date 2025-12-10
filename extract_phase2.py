import os
def read_lines(filepath, start, end):
    with open(filepath, "r") as f:
        lines = f.readlines()
    return "".join(lines[start-1:end])
main_file = "/home/lzz725/FYP/include/dual_pivot_quicksort.hpp"
ranges = {
        'insertion_generic': (1654, 1793),
    "insertion_int": (2413, 2503),
    "insertion_long": (2628, 2715),
    "insertion_float": (2752, 2842),
    "insertion_double": (2967, 3054),
    "counting_byte": (4645, 4701),
    "counting_char": (4703, 4747),
    "counting_short": (4749, 4818),
    "heap_generic": (1795, 1848),
    "heap_int": (2505, 2538),
    "heap_long": (2717, 2750),
    "heap_float": (2844, 2877),
    "heap_double": (3056, 3089)
}
    f.write("#include <vector>
")
    f.write("#include <type_traits>
")
    f.write("#include "utils.hpp"
")
    f.write("#include "constants.hpp"

")
    f.write("namespace dual_pivot {

")
    f.write(read_lines(main_file, *ranges["insertion_generic"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["insertion_int"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["insertion_long"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["insertion_float"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["insertion_double"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["counting_byte"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["counting_char"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["counting_short"]))
    f.write("
} // namespace dual_pivot

")
    f.write("#endif // DPQS_INSERTION_SORT_HPP
")

with open("/home/lzz725/FYP/include/dpqs/heap_sort.hpp", "w") as f:
    f.write("#ifndef DPQS_HEAP_SORT_HPP
")
    f.write("#define DPQS_HEAP_SORT_HPP

")
    f.write("#include "utils.hpp"

")
    f.write("namespace dual_pivot {

")
    f.write(read_lines(main_file, *ranges["heap_generic"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["heap_int"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["heap_long"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["heap_float"]))
    f.write("
")
    f.write(read_lines(main_file, *ranges["heap_double"]))
    f.write("
} // namespace dual_pivot

")
    f.write("#endif // DPQS_HEAP_SORT_HPP
")
