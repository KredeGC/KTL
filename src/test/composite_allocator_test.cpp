#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/composite_allocator.h"
#include "ktl/allocators/freelist_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

namespace ktl
{
    KTL_ADD_TEST(test_composite_stack_freelist_double)
    {
        double values[] = {
                42.5,
                81.3,
                384.6,
                182.1,
                99.9
        };

        stack<16> primaryStack;
        freelist<4096> fallbackStack;
        type_composite_allocator<double, stack_allocator<16>, freelist_allocator<4096>> alloc({ primaryStack, fallbackStack });

        assert_allocate_unordered(alloc, values);
    }

    KTL_ADD_TEST(test_composite_stack_malloc_double)
    {
        double values[] = {
                42.5,
                81.3,
                384.6,
                182.1,
                99.9
        };

        stack<16> stack;
        type_composite_allocator<double, stack_allocator<16>, mallocator> alloc({ stack });

        assert_allocate_unordered(alloc, values);
    }

    KTL_ADD_TEST(test_composite_freelist_double)
    {
        double values[] = {
                42.5,
                81.3,
                384.6,
                182.1,
                99.9
        };

        freelist<32> primaryStack;
        freelist<4096> fallbackStack;
        type_composite_allocator<double, freelist_allocator<32>, freelist_allocator<4096>> alloc({ primaryStack, fallbackStack });

        assert_allocate_unordered(alloc, values);
    }
}