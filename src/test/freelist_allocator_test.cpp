#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/freelist_allocator.h"

// Naming scheme: test_freelist_allocator_[Type]
// Contains tests that relate directly to the ktl::freelist_allocator

namespace ktl
{
    KTL_ADD_HEADER();

    KTL_ADD_TEST(test_freelist_allocator_double)
    {
        freelist<4096> block;
        type_freelist_allocator<double, 4096> alloc(block);
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_freelist_allocator_packed)
    {
        freelist<4096> block;
        type_freelist_allocator<packed_t, 4096> alloc(block);
        assert_unordered_values<packed_t>(alloc);
    }
}