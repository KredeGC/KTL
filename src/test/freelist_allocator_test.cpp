#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/freelist.h"
#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_freelist_allocator_[Type]
// Contains tests that relate directly to the ktl::freelist_allocator

namespace ktl::test::freelist_allocator
{
    KTL_ADD_TEST(test_freelist_stack_allocator_raw_allocate)
    {
        stack<4096> block;
        freelist<0, 8, stack_allocator<4096>> alloc(block);
        assert_raw_allocate_deallocate<1, 2, 4, 4, 8, 8>(alloc);
    }

    KTL_ADD_TEST(test_freelist_stack_allocator_unordered_double)
    {
        stack<4096> block;
        type_freelist_allocator<double, 0, 8, stack_allocator<4096>> alloc(block);
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_freelist_stack_allocator_unordered_packed)
    {
        stack<4096> block;
        type_freelist_allocator<packed_t, 16, 32, stack_allocator<4096>> alloc(block);
        assert_unordered_values<packed_t>(alloc);
    }

    KTL_ADD_TEST(test_freelist_linear_allocator_unordered_packed)
    {
        type_freelist_allocator<packed_t, 16, 32, linear_allocator<4096>> alloc;
        assert_unordered_values<packed_t>(alloc);
    }
}