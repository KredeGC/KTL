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
    template<typename T>
    using Alloc = ktl::type_stack_allocator<T, 4096>;

    KTL_ADD_TEST(test_stack_allocator_raw_allocate)
    {
        stack<4096> block;
        ktl::stack_allocator<4096> alloc(block);
        assert_raw_allocate_deallocate(alloc);
    }

    KTL_ADD_TEST(test_stack_allocator_unordered_double)
    {
        stack<4096> block;
        Alloc<double> alloc(block);
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_stack_allocator_unordered_packed)
    {
        stack<4096> block;
        Alloc<packed_t> alloc(block);
        assert_unordered_values<packed_t>(alloc);
    }

#pragma region std::vector
    KTL_ADD_TEST(test_stack_allocator_std_vector_double)
    {
        stack<4096> block;
        Alloc<double> alloc(ktl::stack_allocator<4096>{ block });
        std::vector<double, Alloc<double>> vec(alloc);
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_stack_allocator_std_vector_trivial)
    {
        stack<4096> block;
        Alloc<trivial_t> alloc(ktl::stack_allocator<4096>{ block });
        std::vector<trivial_t, Alloc<trivial_t>> vec(alloc);
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_stack_allocator_std_vector_packed)
    {
        stack<4096> block;
        Alloc<packed_t> alloc(ktl::stack_allocator<4096>{ block });
        std::vector<packed_t, Alloc<packed_t>> vec(alloc);
        assert_vector_values<packed_t>(vec);
    }

    KTL_ADD_TEST(test_stack_allocator_std_vector_complex)
    {
        stack<4096> block;
        Alloc<complex_t> alloc(ktl::stack_allocator<4096>{ block });
        std::vector<complex_t, Alloc<complex_t>> vec(alloc);
        assert_vector_values<complex_t>(vec);
    }
#pragma endregion
}