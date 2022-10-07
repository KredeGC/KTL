#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/composite_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_composite_[Primary]_[Fallback]_[Container]_[Type]
// Contains tests that relate directly to the ktl::composite_allocator

namespace ktl::test
{
    KTL_ADD_HEADER();

    KTL_ADD_TEST(test_composite_stack_stack_unordered_double)
    {
        stack<16> primaryStack;
        stack<4096> fallbackStack;
        type_composite_allocator<double, stack_allocator<16>, stack_allocator<4096>> alloc({ primaryStack, fallbackStack });
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_composite_stack_pre_unordered_double)
    {
        stack<16> primaryStack;
        arena<4096> fallbackStack;
        type_composite_allocator<double, stack_allocator<16>, pre_allocator<4096>> alloc({ primaryStack, fallbackStack });
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_composite_stack_malloc_unordered_double)
    {
        stack<16> stack;
        type_composite_allocator<double, stack_allocator<16>, mallocator> alloc({ stack });
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_composite_pre_pre_unordered_double)
    {
        arena<32> primaryStack;
        arena<4096> fallbackStack;
        type_composite_allocator<double, pre_allocator<32>, pre_allocator<4096>> alloc({ primaryStack, fallbackStack });
        assert_unordered_values<double>(alloc);
    }
}