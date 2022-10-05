#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_stack_allocator_[Type]
// Contains tests that relate directly to the ktl::stack_allocator

namespace ktl
{
    KTL_ADD_TEST(test_stack_allocator_double)
    {
        stack<4096> block;
        type_stack_allocator<double, 4096> alloc(block);
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_stack_allocator_packed)
    {
        stack<4096> block;
        type_stack_allocator<packed_t, 4096> alloc(block);
        assert_unordered_values<packed_t>(alloc);
    }
}