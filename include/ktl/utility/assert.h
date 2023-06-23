#pragma once

#include <string>

#if defined(_WIN32) // Windows
#define KTL_BREAKPOINT() __debugbreak()
#elif defined(__linux__) // Linux
#include <csignal>
#define KTL_BREAKPOINT() std::raise(SIGTRAP)
#else // Non-supported
#define KTL_BREAKPOINT()
#endif

#ifdef KTL_DEBUG_ASSERT
#define KTL_ASSERT(x) if (!(x)) { KTL_BREAKPOINT(); }
#else
#define KTL_ASSERT(x)
#endif