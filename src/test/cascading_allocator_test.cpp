#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/cascading.h"
#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/segragator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_cascading_[Alloc]_[Container]_[Type]
// Contains tests that relate directly to the ktl::cascading_allocator

namespace ktl::test::cascading_allocator
{
    KTL_ADD_TEST(test_cascading_linear_raw_allocate)
    {
        cascading<linear_allocator<32>> alloc;
        assert_raw_allocate_deallocate<2, 4, 8, 16, 24, 32>(alloc);
    }

    KTL_ADD_TEST(test_cascading_linear_unordered_double)
    {
        type_cascading_allocator<double, linear_allocator<32>> alloc;
        assert_unordered_values<double>(alloc);
    }
}