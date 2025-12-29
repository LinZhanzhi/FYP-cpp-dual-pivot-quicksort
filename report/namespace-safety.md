# Improvement Report: Namespace Safety & Macro Hygiene

## 1. Problem Description
The library defined several preprocessor macros for compiler optimization hints:
- `FORCE_INLINE`
- `LIKELY`
- `UNLIKELY`
- `PREFETCH_READ`

These names are generic and commonly used by other libraries (e.g., Boost, Unreal Engine, Linux kernel headers) or even potentially by the user's own code. Defining them in a public header file (`utils.hpp`) without a unique prefix pollutes the global namespace. This can lead to:
- **Macro Redefinition Warnings**: If the user includes another library that defines the same macros.
- **Compilation Errors**: If definitions conflict or have different signatures.
- **Silent Bugs**: If the compiler uses a different definition than intended.

## 2. Solution Implemented
We renamed all internal macros to include a project-specific prefix: `DPQS_` (Dual-Pivot QuickSort).

### Changes:
| Old Name | New Name | Purpose |
|----------|----------|---------|
| `FORCE_INLINE` | `DPQS_FORCE_INLINE` | Forces function inlining |
| `LIKELY(x)` | `DPQS_LIKELY(x)` | Branch prediction hint (true) |
| `UNLIKELY(x)` | `DPQS_UNLIKELY(x)` | Branch prediction hint (false) |
| `PREFETCH_READ(p)` | `DPQS_PREFETCH_READ(p)` | CPU cache prefetch hint |

### Implementation Details:
- **Definition**: Updated `include/dpqs/utils.hpp` to define the new names.
- **Usage**: Updated all source files in `include/dpqs/` to use the new names.
- **Cleanup**: Removed `include/dpqs/leaf_sorters.hpp`, an unused file that still contained the old macro definitions.

## 3. Impact and Verification
- **Safety**: The library is now "namespace safe" regarding macros. It can be included alongside any other library without fear of collision.
- **Clarity**: The `DPQS_` prefix makes it immediately obvious that these are internal macros belonging to this specific library.
- **Verification**:
    - A `grep` search confirmed that no instances of the old macro names remain in the code (only in documentation).
    - The test suite passed, confirming that the macros are still functioning correctly as optimization hints.
