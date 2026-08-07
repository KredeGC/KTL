#pragma once

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#include <utility>
#define KTL_UNREACHABLE() std::unreachable()
#else

#if defined(_MSC_VER) && !defined(__clang__) // MSVC
#   define KTL_UNREACHABLE() __assume(false);
#else // GCC, Clang
#   define KTL_UNREACHABLE() __builtin_unreachable();
#endif

#endif // __cpp_lib_unreachable