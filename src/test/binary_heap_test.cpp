#include "shared/assert_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/containers/binary_heap.h"

#include "ktl/allocators/freelist_allocator.h"
#include "ktl/allocators/stack_allocator.h"

namespace ktl
{
    KTL_ADD_TEST(test_binary_heap_double)
    {
        constexpr size_t size = 8;

        double values[] = {
            0.0,
            8.0,
            9.0,
            10.0,
            20.0,
            28.0,
            32.0,
            58.0
        };

        freelist<4096> block;
        type_freelist_allocator<double, 4096> alloc(block);
        test_binary_heap_min_max<3>(alloc, values, size);
    }

    KTL_ADD_TEST(test_binary_heap_trivial)
    {
        constexpr size_t size = 8;

        trivial_t values[] = {
            { 0.0f, 0.0f },
            { 8.0f, 7.0f },
            { 10.0f, 9.0f },
            { 9.0f, 11.0f },
            { 20.0f, 3.0f },
            { 32.0f, 8.0f },
            { 28.0f, 24.0f },
            { 58.0f, 31.0f }
        };

        freelist<4096> block;
        type_freelist_allocator<trivial_t, 4096> alloc(block);
        test_binary_heap_min_max<3>(alloc, values, size);
    }

    KTL_ADD_TEST(test_binary_heap_complex)
    {
        constexpr size_t size = 8;

        complex_t values[size] = {
            0.0,
            8.0,
            9.0,
            10.0,
            20.0,
            28.0,
            32.0,
            58.0
        };

        freelist<4096> block;
        type_freelist_allocator<complex_t, 4096> alloc(block);
        test_binary_heap_min_max<3>(alloc, values, size);
    }
}