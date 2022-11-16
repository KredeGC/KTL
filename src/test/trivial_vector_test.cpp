#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/trivial_vector.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_trivial_vector_[Alloc]_[Type]
// Contains tests that relate directly to the ktl::trivial_vector

namespace ktl::test
{
    KTL_ADD_TEST(test_trivial_vector_construct)
    {
        assert_construct_container<trivial_vector<double>>();
    }

    KTL_ADD_TEST(test_trivial_vector_pre_double)
    {
        trivial_vector<double, type_pre_allocator<double, 4096>> vec;
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_trivial_vector_pre_trivial)
    {
        trivial_vector<trivial_t, type_pre_allocator<trivial_t, 4096>> vec;
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_trivial_vector_stack_double)
    {
        stack<4096> block;
        trivial_vector<double, type_stack_allocator<double, 4096>> vec({ block });
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_trivial_vector_stack_trivial)
    {
        stack<4096> block;
        trivial_vector<trivial_t, type_stack_allocator<trivial_t, 4096>> vec({ block });
        assert_vector_values<trivial_t>(vec);
    }
}