#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/linked.h"
#include "ktl/allocators/mallocator.h"

#include <vector>

// Naming scheme: test_linked_allocator_[Type]
// Contains tests that relate directly to the ktl::linked_allocator

namespace ktl::test::linked_allocator
{
    KTL_ADD_TEST(test_linked_mallocator_unordered_double)
    {
        type_linked_allocator<double, 4096, mallocator> alloc;
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_linked_mallocator_unordered_packed)
    {
        type_linked_allocator<packed_t, 4096, mallocator> alloc;
        assert_unordered_values<packed_t>(alloc);
    }

#pragma region std::vector
    KTL_ADD_TEST(test_linked_mallocator_std_vector_double)
    {
        std::vector<double, type_linked_allocator<double, 4096, mallocator>> vec;
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_linked_mallocator_std_vector_trivial)
    {
        std::vector<trivial_t, type_linked_allocator<trivial_t, 4096, mallocator>> vec;
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_linked_mallocator_std_vector_packed)
    {
        std::vector<packed_t, type_linked_allocator<packed_t, 4096, mallocator>> vec;
        assert_vector_values<packed_t>(vec);
    }

    KTL_ADD_TEST(test_linked_mallocator_std_vector_complex)
    {
        std::vector<complex_t, type_linked_allocator<complex_t, 4096, mallocator>> vec;
        assert_vector_values<complex_t>(vec);
    }
#pragma endregion
}