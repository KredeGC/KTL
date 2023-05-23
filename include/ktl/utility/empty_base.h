#pragma once

#if defined(__has_cpp_attribute) && __has_cpp_attribute(no_unique_address)
#define KTL_EMPTY_BASE [[no_unique_address]]
#else // __has_cpp_attribute(no_unique_address)
#define KTL_EMPTY_BASE
#endif // __has_cpp_attribute(no_unique_address)