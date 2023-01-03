#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/segragator.h"
#include "ktl/allocators/stack_allocator.h"

#include <iostream>

// Naming scheme: test_segragator_[Primary]_[Fallback]_[Container]_[Type]
// Contains tests that relate directly to the ktl::segragator_allocator

namespace ktl::test::segragator_allocator
{
    KTL_ADD_TEST(test_segragator_variadic)
    {
        // Autogenerated allocator, which should use exactly 2 comparisons, since the tree is full
        using Alloc1 = segragator_builder_t<
            linear_allocator<1024>,
            threshold<8>,
            linear_allocator<2048>,
            threshold<16>,
            linear_allocator<4096>,
            threshold<32>,
            mallocator>;
        
        // This allocator might use up to 3 comparisons, since the tree isn't complete
        using Alloc2 = segragator<8,
            linear_allocator<1024>,
            segragator<16,
                linear_allocator<2048>,
                segragator<32,
                    linear_allocator<4096>,
                    mallocator>>>;

        // More complex allocator where the tree isn't full
        using Alloc3 = segragator_builder_t<
            linear_allocator<1024>,
            threshold<8>,
            linear_allocator<2048>,
            threshold<16>,
            linear_allocator<4096>,
            threshold<32>,
            linear_allocator<8192>,
            threshold<48>,
            mallocator>;

        // Same allocator as above, but written manually, where you can see the tree structure
        using Alloc4 = segragator<8,
            linear_allocator<1024>,
            segragator<32,
                segragator<16,
                    linear_allocator<2048>,
                    linear_allocator<4096>>,
                segragator<48,
                    linear_allocator<8192>,
                    mallocator>>>;

        Alloc1 alloc1;
        Alloc2 alloc2;
        Alloc3 alloc3;
        Alloc4 alloc4;

        static_assert(has_no_value_type<Alloc1>::value);
        static_assert(has_no_value_type<Alloc2>::value);
        static_assert(has_no_value_type<Alloc3>::value);
        static_assert(has_no_value_type<Alloc4>::value);
        
        static_assert(!std::is_same_v<Alloc1, Alloc2>, "The allocator types shouldn't match");
        static_assert(std::is_same_v<Alloc3, Alloc4>, "The allocator types don't match");
    }
    
    KTL_ADD_TEST(test_segragator_stack_stack_unordered_double)
    {
        using AllocDouble = type_segragator_allocator<double, 8, stack_allocator<1024>, stack_allocator<1024>>;
        using AllocTrivial = typename std::allocator_traits<AllocDouble>::template rebind_alloc<trivial_t>;

        stack<1024> primaryStack;
        stack<1024> fallbackStack;
        AllocDouble double_alloc({ primaryStack, fallbackStack });

        assert_unordered_values<double>(double_alloc);

        AllocTrivial trivial_alloc = static_cast<AllocTrivial>(double_alloc);
        assert_unordered_values<trivial_t>(trivial_alloc);
    }
}