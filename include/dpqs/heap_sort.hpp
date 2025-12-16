#ifndef DPQS_HEAP_SORT_HPP
#define DPQS_HEAP_SORT_HPP

#include "utils.hpp"

namespace dual_pivot {

static void pushDown_int(int* array, int parent_index, int value, int offset, int upper_bound);
static void pushDown_long(long* array, int parent_index, long value, int offset, int upper_bound);
static void pushDown_float(float* array, int parent_index, float value, int offset, int upper_bound);
static void pushDown_double(double* array, int parent_index, double value, int offset, int upper_bound);
// static void pushDown_byte(signed char* a, int p, signed char value, int low, int high);
// static void pushDown_char(char* a, int p, char value, int low, int high);
// static void pushDown_short(short* a, int p, short value, int low, int high);

/**
 * @brief Restores the heap property by sifting a node down the heap.
 *
 * @tparam T The type of elements in the array.
 * @param array The array containing the heap.
 * @param parent_index The index of the node to sift down.
 * @param value The value of the node being sifted.
 * @param offset The start index of the heap in the array.
 * @param upper_bound The exclusive upper bound of the heap in the array.
 */
template<typename T>
void pushDown(T* array, int parent_index, T value, int offset, int upper_bound) {
    for (int child_index;;) {
        // Calculate the index of the right child.
        // The heap structure is implicit in the array range [offset, upper_bound].
        // The formula maps the 0-based heap index to the array index.
        child_index = (parent_index << 1) - offset + 2;

        // If the child index is out of bounds, we've reached a leaf.
        if (child_index > upper_bound) {
            break;
        }
        // Compare the right child with the left child (child_index - 1).
        // If the right child is out of bounds (child_index == upper_bound) or smaller than the left child,
        // we select the left child as the candidate for swapping.
        if (child_index == upper_bound || array[child_index] < array[child_index - 1]) {
            --child_index;
        }
        // If the larger child is smaller than or equal to the value being sifted down,
        // the heap property is satisfied, and we can stop.
        if (array[child_index] <= value) {
            break;
        }
        // Move the larger child up to the parent's position.
        array[parent_index] = array[child_index];
        // Update the parent index to the child's index and continue sifting down.
        parent_index = child_index;
    }
    // Place the value in its correct position.
    array[parent_index] = value;
}

/**
 * @brief Sorts a range of the array using Heap Sort.
 *
 * @tparam T The type of elements in the array.
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
template<typename T>
void heapSort(T* array, int start_index, int end_index) {
    // Phase 1: Build the heap.
    // Start from the last non-leaf node and sift down each node to establish the heap property.
    for (int node_index = (start_index + end_index) >> 1; node_index > start_index; ) {
        --node_index;
        pushDown(array, node_index, array[node_index], start_index, end_index);
    }
    // Phase 2: Sort the array.
    // Repeatedly extract the maximum element (at start_index) and place it at the end of the current range.
    // Then reduce the range and restore the heap property.
    while (--end_index > start_index) {
        T max = array[start_index];
        pushDown(array, start_index, array[end_index], start_index, end_index);
        array[end_index] = max;
    }
}

/**
 * @brief Sorts a range of an integer array using Heap Sort.
 *
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
static void heapSort_int(int* array, int start_index, int end_index) {
    // Phase 1: Build the heap.
    for (int node_index = (start_index + end_index) >> 1; node_index > start_index; ) {
        --node_index;
        pushDown_int(array, node_index, array[node_index], start_index, end_index);
    }
    // Phase 2: Sort the array.
    while (--end_index > start_index) {
        int max = array[start_index];
        pushDown_int(array, start_index, array[end_index], start_index, end_index);
        array[end_index] = max;
    }
}

/**
 * @brief Restores the heap property for an integer array.
 *
 * @param array The array containing the heap.
 * @param parent_index The index of the node to sift down.
 * @param value The value of the node being sifted.
 * @param offset The start index of the heap in the array.
 * @param upper_bound The exclusive upper bound of the heap in the array.
 */
static void pushDown_int(int* array, int parent_index, int value, int offset, int upper_bound) {
    for (int child_index;;) {
        // Calculate right child index
        child_index = (parent_index << 1) - offset + 2;

        if (child_index > upper_bound) {
            break;
        }
        // Select the larger child
        if (child_index == upper_bound || array[child_index] < array[child_index - 1]) {
            --child_index;
        }
        // Check if heap property is satisfied
        if (array[child_index] <= value) {
            break;
        }
        // Move child up
        array[parent_index] = array[child_index];
        parent_index = child_index;
    }
    array[parent_index] = value;
}

/**
 * @brief Sorts a range of a long integer array using Heap Sort.
 *
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
static void heapSort_long(long* array, int start_index, int end_index) {
    // Phase 1: Build the heap.
    for (int node_index = (start_index + end_index) >> 1; node_index > start_index; ) {
        --node_index;
        pushDown_long(array, node_index, array[node_index], start_index, end_index);
    }
    // Phase 2: Sort the array.
    while (--end_index > start_index) {
        long max = array[start_index];
        pushDown_long(array, start_index, array[end_index], start_index, end_index);
        array[end_index] = max;
    }
}

/**
 * @brief Restores the heap property for a long integer array.
 *
 * @param array The array containing the heap.
 * @param parent_index The index of the node to sift down.
 * @param value The value of the node being sifted.
 * @param offset The start index of the heap in the array.
 * @param upper_bound The exclusive upper bound of the heap in the array.
 */
static void pushDown_long(long* array, int parent_index, long value, int offset, int upper_bound) {
    for (int child_index;;) {
        // Calculate right child index
        child_index = (parent_index << 1) - offset + 2;

        if (child_index > upper_bound) {
            break;
        }
        // Select the larger child
        if (child_index == upper_bound || array[child_index] < array[child_index - 1]) {
            --child_index;
        }
        // Check if heap property is satisfied
        if (array[child_index] <= value) {
            break;
        }
        // Move child up
        array[parent_index] = array[child_index];
        parent_index = child_index;
    }
    array[parent_index] = value;
}

/**
 * @brief Sorts a range of a float array using Heap Sort.
 *
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
static void heapSort_float(float* array, int start_index, int end_index) {
    // Phase 1: Build the heap.
    for (int node_index = (start_index + end_index) >> 1; node_index > start_index; ) {
        --node_index;
        pushDown_float(array, node_index, array[node_index], start_index, end_index);
    }
    // Phase 2: Sort the array.
    while (--end_index > start_index) {
        float max = array[start_index];
        pushDown_float(array, start_index, array[end_index], start_index, end_index);
        array[end_index] = max;
    }
}

/**
 * @brief Restores the heap property for a float array.
 *
 * @param array The array containing the heap.
 * @param parent_index The index of the node to sift down.
 * @param value The value of the node being sifted.
 * @param offset The start index of the heap in the array.
 * @param upper_bound The exclusive upper bound of the heap in the array.
 */
static void pushDown_float(float* array, int parent_index, float value, int offset, int upper_bound) {
    for (int child_index;;) {
        // Calculate right child index
        child_index = (parent_index << 1) - offset + 2;

        if (child_index > upper_bound) {
            break;
        }
        // Select the larger child
        if (child_index == upper_bound || array[child_index] < array[child_index - 1]) {
            --child_index;
        }
        // Check if heap property is satisfied
        if (array[child_index] <= value) {
            break;
        }
        // Move child up
        array[parent_index] = array[child_index];
        parent_index = child_index;
    }
    array[parent_index] = value;
}

/**
 * @brief Sorts a range of a double array using Heap Sort.
 *
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
static void heapSort_double(double* array, int start_index, int end_index) {
    // Phase 1: Build the heap.
    for (int node_index = (start_index + end_index) >> 1; node_index > start_index; ) {
        --node_index;
        pushDown_double(array, node_index, array[node_index], start_index, end_index);
    }
    // Phase 2: Sort the array.
    while (--end_index > start_index) {
        double max = array[start_index];
        pushDown_double(array, start_index, array[end_index], start_index, end_index);
        array[end_index] = max;
    }
}

/**
 * @brief Restores the heap property for a double array.
 *
 * @param array The array containing the heap.
 * @param parent_index The index of the node to sift down.
 * @param value The value of the node being sifted.
 * @param offset The start index of the heap in the array.
 * @param upper_bound The exclusive upper bound of the heap in the array.
 */
static void pushDown_double(double* array, int parent_index, double value, int offset, int upper_bound) {
    for (int child_index;;) {
        // Calculate right child index
        child_index = (parent_index << 1) - offset + 2;

        if (child_index > upper_bound) {
            break;
        }
        // Select the larger child
        if (child_index == upper_bound || array[child_index] < array[child_index - 1]) {
            --child_index;
        }
        // Check if heap property is satisfied
        if (array[child_index] <= value) {
            break;
        }
        // Move child up
        array[parent_index] = array[child_index];
        parent_index = child_index;
    }
    array[parent_index] = value;
}

} // namespace dual_pivot

#endif // DPQS_HEAP_SORT_HPP
