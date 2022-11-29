#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/cascading_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/segragator_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_cascading_[Alloc]_[Container]_[Type]
// Contains tests that relate directly to the ktl::cascading_allocator

namespace ktl::test
{
    KTL_ADD_TEST(test_cascading_pre_unordered_double)
    {
        type_cascading_allocator<double, pre_allocator<32>> alloc;
        assert_unordered_values<double>(alloc);
    }
}