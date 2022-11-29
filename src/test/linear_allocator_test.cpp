#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/linear_allocator.h"

#include <vector>

// Naming scheme: test_linear_allocator_[Type]
// Contains tests that relate directly to the ktl::linear_allocator

namespace ktl::test
{
    KTL_ADD_TEST(test_linear_allocator_unordered_double)
    {
        type_linear_allocator<double, 4096> alloc;
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_linear_allocator_unordered_packed)
    {
        type_linear_allocator<packed_t, 4096> alloc;
        assert_unordered_values<packed_t>(alloc);
    }

#pragma region std::vector
    KTL_ADD_TEST(test_linear_allocator_std_vector_double)
    {
        std::vector<double, type_linear_allocator<double, 4096>> vec;
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_linear_allocator_std_vector_trivial)
    {
        std::vector<trivial_t, type_linear_allocator<trivial_t, 4096>> vec;
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_linear_allocator_std_vector_packed)
    {
        std::vector<packed_t, type_linear_allocator<packed_t, 4096>> vec;
        assert_vector_values<packed_t>(vec);
    }

    KTL_ADD_TEST(test_linear_allocator_std_vector_complex)
    {
        std::vector<complex_t, type_linear_allocator<complex_t, 4096>> vec;
        assert_vector_values<complex_t>(vec);
    }
#pragma endregion
}