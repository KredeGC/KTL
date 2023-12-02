#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/reference.h"
#include "ktl/allocators/type_allocator.h"

#include <vector>

// Naming scheme: test_reference_allocator_[Type]
// Contains tests that relate directly to the ktl::reference

namespace ktl::test::reference_allocator
{
    KTL_ADD_TEST(test_reference_allocator_construct)
    {
        ktl::linear_allocator<4096> alloc;

        ktl::reference<ktl::linear_allocator<4096>> ref(alloc);

        ktl::reference<ktl::linear_allocator<4096>> moved_ref(std::move(ref));
    }

    KTL_ADD_TEST(test_reference_allocator_raw_allocate)
    {
        ktl::linear_allocator<4096> alloc;

        ktl::reference<ktl::linear_allocator<4096>> ref(alloc);

        assert_raw_allocate_deallocate<2, 4, 8, 16, 32, 64>(ref);
    }

#pragma region std::vector
    KTL_ADD_TEST(test_reference_allocator_std_vector_double)
    {
        ktl::linear_allocator<4096> alloc;
        ktl::type_reference_linear_allocator<double, 4096> ref_alloc(alloc);
        std::vector<double, type_reference_linear_allocator<double, 4096>> vec(ref_alloc);
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_reference_allocator_std_vector_trivial)
    {
        ktl::linear_allocator<4096> alloc;
        ktl::type_reference_linear_allocator<trivial_t, 4096> ref_alloc(alloc);
        std::vector<trivial_t, type_reference_linear_allocator<trivial_t, 4096>> vec(ref_alloc);
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_reference_allocator_std_vector_packed)
    {
        ktl::linear_allocator<4096> alloc;
        ktl::type_reference_linear_allocator<packed_t, 4096> ref_alloc(alloc);
        std::vector<packed_t, type_reference_linear_allocator<packed_t, 4096>> vec(ref_alloc);
        assert_vector_values<packed_t>(vec);
    }

    KTL_ADD_TEST(test_reference_allocator_std_vector_complex)
    {
        ktl::linear_allocator<4096> alloc;
        ktl::type_reference_linear_allocator<complex_t, 4096> ref_alloc(alloc);
        std::vector<complex_t, type_reference_linear_allocator<complex_t, 4096>> vec(ref_alloc);
        assert_vector_values<complex_t>(vec);
    }
#pragma endregion
}