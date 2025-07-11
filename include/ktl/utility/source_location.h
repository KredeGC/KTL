#pragma once

#include <cstdint>

#if __cpp_lib_source_location >= 201907L
#define KTL_SOURCE_LOCATION
#include <source_location>

#define KTL_SOURCE() std::source_location::current()
#else
#define KTL_SOURCE() {}
#endif

namespace ktl
{
#ifdef KTL_SOURCE_LOCATION
    using source_location = std::source_location;
#else
    struct source_location {};
#endif
}