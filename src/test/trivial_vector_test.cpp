#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/trivial_vector.h"

#include "ktl/allocators/list_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_trivial_vector_[Alloc]_[Type]
// Contains tests that relate directly to the ktl::trivial_vector

namespace ktl::test::trivial_vector
{
    KTL_ADD_TEST(test_trivial_vector_construct)
    {
        assert_construct_container<ktl::trivial_vector<double>>();
    }

    KTL_ADD_TEST(test_trivial_vector_list_double)
    {
        ktl::trivial_vector<double, type_list_allocator<double, 4096>> vec;
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_trivial_vector_list_trivial)
    {
        ktl::trivial_vector<trivial_t, type_list_allocator<trivial_t, 4096>> vec;
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_trivial_vector_stack_double)
    {
        using Alloc = ktl::type_stack_allocator<double, 4096>;

        stack<4096> block;
        Alloc alloc(ktl::stack_allocator<4096>{ block });
        ktl::trivial_vector<double, Alloc> vec(alloc);
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_trivial_vector_stack_trivial)
    {
        using Alloc = ktl::type_stack_allocator<trivial_t, 4096>;

        stack<4096> block;
        Alloc alloc(ktl::stack_allocator<4096>{ block });
        ktl::trivial_vector<trivial_t, Alloc> vec(alloc);
        assert_vector_values<trivial_t>(vec);
    }
}