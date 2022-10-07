#pragma once

#include <cstddef>
#include <memory>

// KTL_MALLOC_ALREADY_ALIGNED
#if defined(__GLIBC__) && ((__GLIBC__ >= 2 && __GLIBC_MINOR__ >= 8) || __GLIBC__ > 2) && defined(__LP64__)
#define KTL_GLIBC_MALLOC_ALREADY_ALIGNED 1
#else
#define KTL_GLIBC_MALLOC_ALREADY_ALIGNED 0
#endif

#if defined(__FreeBSD__) && !defined(__arm__) && !defined(__mips__)
#define KTL_FREEBSD_MALLOC_ALREADY_ALIGNED 1
#else
#define KTL_FREEBSD_MALLOC_ALREADY_ALIGNED 0
#endif

#if (defined(__APPLE__) \
 || defined(_WIN64) \
 || KTL_GLIBC_MALLOC_ALREADY_ALIGNED \
 || KTL_FREEBSD_MALLOC_ALREADY_ALIGNED)
#define KTL_HAS_MALLOC_ALIGNED 1
#else
#define KTL_HAS_MALLOC_ALIGNED 0
#endif
// KTL_MALLOC_ALREADY_ALIGNED

// KTL_HAS_POSIX_MEMALIGN
#if ((defined __QNXNTO__) || (defined _GNU_SOURCE) || ((defined _XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600))) \
 && (defined _POSIX_ADVISORY_INFO) && (_POSIX_ADVISORY_INFO > 0)
#define KTL_HAS_POSIX_MEMALIGN 1
#else
#define KTL_HAS_POSIX_MEMALIGN 0
#endif
// KTL_HAS_POSIX_MEMALIGN

// KTL_HAS_MM_MALLOC
#if SSE_INSTR_SET > 0
#define KTL_HAS_MM_MALLOC 1
#else
#define KTL_HAS_MM_MALLOC 0
#endif
// KTL_HAS_MM_MALLOC

namespace ktl
{
    inline void* aligned_malloc(size_t size, size_t alignment)
    {
#if KTL_HAS_MALLOC_ALIGNED
        return malloc(size);
#elif KTL_HAS_MM_MALLOC
        return _mm_malloc(size, alignment);
#elif KTL_HAS_POSIX_MEMALIGN
        void* res;
        const int failed = posix_memalign(&res, size, alignment);
        if (failed) res = nullptr;
        return res;
#elif defined(_MSC_VER)
        return _aligned_malloc(size, alignment);
#else
        void* res = nullptr;
        void* ptr = malloc(size + alignment);
        if (ptr != nullptr)
        {
            res = reinterpret_cast<void*>((reinterpret_cast<size_t>(ptr) & ~(size_t(alignment - 1))) + alignment);
            *(reinterpret_cast<void**>(res) - 1) = ptr;
        }
        return res;
#endif
    }

    inline void aligned_free(void* ptr)
    {
#if KTL_HAS_MALLOC_ALIGNED
        free(ptr);
#elif KTL_HAS_MM_MALLOC
        _mm_free(ptr);
#elif KTL_HAS_POSIX_MEMALIGN
        free(ptr);
#elif defined(_MSC_VER)
        _aligned_free(ptr);
#else
        if (ptr != 0)
            free(*(reinterpret_cast<void**>(ptr) - 1));
#endif
    }
}