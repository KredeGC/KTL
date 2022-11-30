#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/stack_allocator.h"

#include <vector>

// Naming scheme: test_stack_allocator_[Type]
// Contains tests that relate directly to the ktl::stack_allocator

namespace ktl::test::stack_allocator
{
    KTL_ADD_TEST(test_stack_allocator_unordered_double)
    {
        stack<4096> block;
        type_stack_allocator<double, 4096> alloc(block);
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_stack_allocator_unordered_packed)
    {
        stack<4096> block;
        type_stack_allocator<packed_t, 4096> alloc(block);
        assert_unordered_values<packed_t>(alloc);
    }

#pragma region std::vector
    KTL_ADD_TEST(test_stack_allocator_std_vector_double)
    {
        stack<4096> block;
        std::vector<double, type_stack_allocator<double, 4096>> vec({ block });
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_stack_allocator_std_vector_trivial)
    {
        stack<4096> block;
        std::vector<trivial_t, type_stack_allocator<trivial_t, 4096>> vec({ block });
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_stack_allocator_std_vector_packed)
    {
        stack<4096> block;
        std::vector<packed_t, type_stack_allocator<packed_t, 4096>> vec({ block });
        assert_vector_values<packed_t>(vec);
    }

    KTL_ADD_TEST(test_stack_allocator_std_vector_complex)
    {
        stack<4096> block;
        std::vector<complex_t, type_stack_allocator<complex_t, 4096>> vec({ block });
        assert_vector_values<complex_t>(vec);
    }
#pragma endregion
}