#ifndef DPQS_TYPES_HPP
#define DPQS_TYPES_HPP

#include <variant>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <cstdint>

namespace dual_pivot {

/**
 * @brief Type-erased array variant for generic array operations
 *
 * This variant type allows the algorithm to work with different primitive types
 * in a type-safe manner, similar to Java's Object[] arrays. It supports all
 * primitive types that benefit from dual-pivot quicksort optimizations.
 *
 * The variant approach provides better type safety than void* while maintaining
 * the flexibility needed for generic array operations in the sorting implementation.
 */
using ArrayVariant = std::variant<
    int*, long*, float*, double*,           // Main numeric types
    signed char*, char*, short*,            // Smaller integer types
    unsigned char*, unsigned short*        // Unsigned variants
>;

/**
 * @brief Array pointer wrapper for type-safe array operations
 *
 * This wrapper class provides a type-safe interface for working with arrays of different
 * primitive types, similar to Java's Object array handling but with compile-time type safety.
 * It encapsulates the array pointer along with size and element size information.
 *
 * The class provides type checking methods (equivalent to Java's instanceof) and
 * type extraction methods (equivalent to Java's casting) to enable generic algorithms
 * while maintaining type safety.
 */
struct ArrayPointer {
    ArrayVariant data;      ///< The actual array pointer stored as a variant
    int size;              ///< Number of elements in the array
    int element_size;      ///< Size of each element in bytes

    /**
     * @brief Default constructor creating an empty ArrayPointer
     */
    ArrayPointer() : data(static_cast<int*>(nullptr)), size(0), element_size(0) {}

    /**
     * @brief Template constructor for creating ArrayPointer from typed array
     * @tparam T The element type of the array
     * @param ptr Pointer to the array
     * @param sz Size of the array (default: 0)
     */
    template<typename T>
    ArrayPointer(T* ptr, int sz = 0) : data(ptr), size(sz), element_size(sizeof(T)) {}

    // Type checking methods (equivalent to Java's instanceof operator)

    /**
     * @brief Check if the array contains int elements
     * @return true if the array is of type int*
     */
    bool isIntArray() const { return std::holds_alternative<int*>(data); }

    /**
     * @brief Check if the array contains long elements
     * @return true if the array is of type long*
     */
    bool isLongArray() const { return std::holds_alternative<long*>(data); }

    /**
     * @brief Check if the array contains float elements
     * @return true if the array is of type float*
     */
    bool isFloatArray() const { return std::holds_alternative<float*>(data); }

    /**
     * @brief Check if the array contains double elements
     * @return true if the array is of type double*
     */
    bool isDoubleArray() const { return std::holds_alternative<double*>(data); }

    /**
     * @brief Check if the array contains signed char elements
     * @return true if the array is of type signed char*
     */
    bool isByteArray() const { return std::holds_alternative<signed char*>(data); }

    /**
     * @brief Check if the array contains char elements
     * @return true if the array is of type char*
     */
    bool isCharArray() const { return std::holds_alternative<char*>(data); }

    /**
     * @brief Check if the array contains short elements
     * @return true if the array is of type short*
     */
    bool isShortArray() const { return std::holds_alternative<short*>(data); }

    /**
     * @brief Check if the array contains unsigned char elements
     * @return true if the array is of type unsigned char*
     */
    bool isUnsignedByteArray() const { return std::holds_alternative<unsigned char*>(data); }

    /**
     * @brief Check if the array contains unsigned short elements
     * @return true if the array is of type unsigned short*
     */
    bool isUnsignedShortArray() const { return std::holds_alternative<unsigned short*>(data); }

    // Type extraction methods (equivalent to Java's casting operations)

    /**
     * @brief Extract int array pointer
     * @return Pointer to int array
     * @throws std::bad_variant_access if the array is not of type int*
     */
    int* asIntArray() const { return std::get<int*>(data); }

    /**
     * @brief Extract long array pointer
     * @return Pointer to long array
     * @throws std::bad_variant_access if the array is not of type long*
     */
    long* asLongArray() const { return std::get<long*>(data); }

    /**
     * @brief Extract float array pointer
     * @return Pointer to float array
     * @throws std::bad_variant_access if the array is not of type float*
     */
    float* asFloatArray() const { return std::get<float*>(data); }

    /**
     * @brief Extract double array pointer
     * @return Pointer to double array
     * @throws std::bad_variant_access if the array is not of type double*
     */
    double* asDoubleArray() const { return std::get<double*>(data); }

    /**
     * @brief Extract signed char array pointer
     * @return Pointer to signed char array
     * @throws std::bad_variant_access if the array is not of type signed char*
     */
    signed char* asByteArray() const { return std::get<signed char*>(data); }

    /**
     * @brief Extract char array pointer
     * @return Pointer to char array
     * @throws std::bad_variant_access if the array is not of type char*
     */
    char* asCharArray() const { return std::get<char*>(data); }

    /**
     * @brief Extract short array pointer
     * @return Pointer to short array
     * @throws std::bad_variant_access if the array is not of type short*
     */
    short* asShortArray() const { return std::get<short*>(data); }

    /**
     * @brief Extract unsigned char array pointer
     * @return Pointer to unsigned char array
     * @throws std::bad_variant_access if the array is not of type unsigned char*
     */
    unsigned char* asUnsignedByteArray() const { return std::get<unsigned char*>(data); }

    /**
     * @brief Extract unsigned short array pointer
     * @return Pointer to unsigned short array
     * @throws std::bad_variant_access if the array is not of type unsigned short*
     */
    unsigned short* asUnsignedShortArray() const { return std::get<unsigned short*>(data); }

    /**
     * @brief Generic visitor pattern for type dispatch
     *
     * This method allows applying operations to the stored array regardless of its type
     * by using the visitor pattern. The visitor function will be called with the actual
     * typed pointer.
     *
     * @tparam Visitor A callable that can handle all possible array types
     * @param visitor The visitor function to apply
     * @return The result of the visitor function
     */
    template<typename Visitor>
    auto visit(Visitor&& visitor) const {
        return std::visit(std::forward<Visitor>(visitor), data);
    }
};

/**
 * @brief Factory function for creating ArrayPointer instances
 *
 * Convenience function to create ArrayPointer objects with proper type deduction.
 *
 * @tparam T The element type of the array
 * @param ptr Pointer to the array
 * @param size Size of the array (default: 0)
 * @return ArrayPointer wrapping the given array
 */
template<typename T>
ArrayPointer makeArrayPointer(T* ptr, int size = 0) {
    return ArrayPointer(ptr, size);
}

} // namespace dual_pivot

#endif // DPQS_TYPES_HPP
