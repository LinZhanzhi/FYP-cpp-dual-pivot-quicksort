#ifndef DPQS_HEAP_SORT_HPP
#define DPQS_HEAP_SORT_HPP

#include "utils.hpp"

namespace dual_pivot {

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
void push_down(T* array, int parent_index, T value, int offset, int upper_bound) {
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
 * This implementation is used as a fallback in Dual-Pivot Quicksort when the recursion depth
 * becomes too large, preventing worst-case O(n^2) behavior (Introsort strategy).
 *
 * @tparam T The type of elements in the array.
 * @param array The array to sort.
 * @param start_index The inclusive start index of the range.
 * @param end_index The exclusive end index of the range.
 */
template<typename T>
void heap_sort(T* array, int start_index, int end_index) {
    // Phase 1: Build the heap.
    // Start from the last non-leaf node and sift down each node to establish the heap property.
    for (int node_index = (start_index + end_index) >> 1; node_index > start_index; ) {
        --node_index;
        push_down(array, node_index, array[node_index], start_index, end_index);
    }
    // Phase 2: Sort the array.
    // Repeatedly extract the maximum element (at start_index) and place it at the end of the current range.
    // Then reduce the range and restore the heap property.
    while (--end_index > start_index) {
        T max = array[start_index];
        push_down(array, start_index, array[end_index], start_index, end_index);
        array[end_index] = max;
    }
}

} // namespace dual_pivot

#endif // DPQS_HEAP_SORT_HPP
