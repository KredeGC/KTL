#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/segragator_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_segragator_[Primary]_[Fallback]_[Container]_[Type]
// Contains tests that relate directly to the ktl::segragator_allocator

namespace ktl::test
{
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