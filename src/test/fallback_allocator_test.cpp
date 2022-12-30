#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/fallback.h"
#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/linked.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_fallback_[Primary]_[Fallback]_[Container]_[Type]
// Contains tests that relate directly to the ktl::fallback_allocator

namespace ktl::test::fallback_allocator
{
    KTL_ADD_TEST(test_fallback_variadic)
    {
        using Alloc1 = fallback_builder_t<
            linear_allocator<1024>,
            linear_allocator<2048>,
            linear_allocator<4096>,
            mallocator>;
        
        using Alloc2 = ktl::fallback<
            linear_allocator<1024>,
            fallback<
                linear_allocator<2048>,
                fallback<
                    linear_allocator<4096>,
                    mallocator>>>;
        
        static_assert(std::is_same_v<Alloc1, Alloc2>, "The allocator types don't match");
    }
    
    KTL_ADD_TEST(test_fallback_stack_stack_unordered_double)
    {
        stack<16> primaryStack;
        stack<4096> fallbackStack;
        type_fallback_allocator<double, stack_allocator<16>, stack_allocator<4096>> alloc({ primaryStack, fallbackStack });
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_fallback_stack_list_unordered_double)
    {
        constexpr size_t size = list_min_size<4096, linear_allocator<4096>>::value;
        
        stack<16> primaryStack;
        type_fallback_allocator<double, stack_allocator<16>, linked<4096, linear_allocator<size>>> alloc({ primaryStack });
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_fallback_stack_malloc_unordered_double)
    {
        stack<16> stack;
        type_fallback_allocator<double, stack_allocator<16>, mallocator> alloc({ stack });
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_fallback_list_list_unordered_double)
    {
        type_fallback_allocator<double, linked<64, mallocator>, linked<4096, mallocator>> alloc;
        assert_unordered_values<double>(alloc);
    }
}