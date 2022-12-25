#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/fallback.h"
#include "ktl/allocators/list_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_fallback_[Primary]_[Fallback]_[Container]_[Type]
// Contains tests that relate directly to the ktl::fallback_allocator

namespace ktl::test::fallback_allocator
{
    KTL_ADD_TEST(test_fallback_stack_stack_unordered_double)
    {
        stack<16> primaryStack;
        stack<4096> fallbackStack;
        type_fallback_allocator<double, stack_allocator<16>, stack_allocator<4096>> alloc({ primaryStack, fallbackStack });
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_fallback_stack_pre_unordered_double)
    {
        stack<16> primaryStack;
        type_fallback_allocator<double, stack_allocator<16>, list_allocator<4096>> alloc({ primaryStack });
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_fallback_stack_malloc_unordered_double)
    {
        stack<16> stack;
        type_fallback_allocator<double, stack_allocator<16>, mallocator> alloc({ stack });
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_fallback_pre_pre_unordered_double)
    {
        type_fallback_allocator<double, list_allocator<32>, list_allocator<4096>> alloc;
        assert_unordered_values<double>(alloc);
    }
}