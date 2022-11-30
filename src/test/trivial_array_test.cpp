#include "shared/array_utility.h"
#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/trivial_array.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_trivial_array_[Alloc]_[Type]
// Contains tests that relate directly to the ktl::trivial_array

namespace ktl::test::trivial_array
{
    KTL_ADD_TEST(test_trivial_array_construct)
    {
        assert_construct_container<ktl::trivial_array<double>>();
    }

    KTL_ADD_TEST(test_trivial_array_pre_double)
    {
        ktl::trivial_array<double, type_pre_allocator<double, 4096>> arr;

        assert_array_values<double>(arr);
    }

    KTL_ADD_TEST(test_trivial_array_pre_trivial)
    {
        ktl::trivial_array<trivial_t, type_pre_allocator<trivial_t, 4096>> arr;

        assert_array_values<trivial_t>(arr);
    }

    KTL_ADD_TEST(test_trivial_array_stack_double)
    {
        using Alloc = ktl::type_stack_allocator<double, 4096>;

        stack<4096> block;
        Alloc alloc(ktl::stack_allocator<4096>{ block });
        ktl::trivial_array<double, Alloc> arr(alloc);
        assert_array_values<double>(arr);
    }

    KTL_ADD_TEST(test_trivial_array_stack_trivial)
    {
        using Alloc = ktl::type_stack_allocator<trivial_t, 4096>;

        stack<4096> block;
        Alloc alloc(ktl::stack_allocator<4096>{ block });
        ktl::trivial_array<trivial_t, Alloc> arr(alloc);
        assert_array_values<trivial_t>(arr);
    }
}