#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/global.h"
#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/reference.h"
#include "ktl/allocators/type_allocator.h"

#include <vector>

// Naming scheme: test_global_allocator_[Type]
// Contains tests that relate directly to the ktl::global

namespace ktl::test::global_allocator
{
    template<typename T>
    using Alloc = ktl::type_allocator<T, ktl::global<ktl::linear_allocator<4096>>>;

    KTL_ADD_TEST(test_global_allocator_equality)
    {
        Alloc<double> alloc1;
        Alloc<trivial_t> alloc2;
        ktl::reference<ktl::global<ktl::linear_allocator<4096>>> alloc3(alloc2.get_allocator());

        KTL_TEST_ASSERT(alloc1 == alloc2);
        KTL_TEST_ASSERT(alloc2.get_allocator() == alloc3.get_allocator());
    }

    KTL_ADD_TEST(test_global_allocator_raw_allocate)
    {
        ktl::global<ktl::linear_allocator<4096>> alloc;

        assert_raw_allocate_deallocate<2, 4, 8, 16, 32, 64>(alloc);
    }

#pragma region std::vector
    KTL_ADD_TEST(test_global_allocator_std_vector_double)
    {
        Alloc<double> alloc;
        std::vector<double, Alloc<double>> vec(alloc);
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_global_allocator_std_vector_trivial)
    {
        Alloc<trivial_t> alloc;
        std::vector<trivial_t, Alloc<trivial_t>> vec(alloc);
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_global_allocator_std_vector_packed)
    {
        Alloc<packed_t> alloc;
        std::vector<packed_t, Alloc<packed_t>> vec(alloc);
        assert_vector_values<packed_t>(vec);
    }

    KTL_ADD_TEST(test_global_allocator_std_vector_complex)
    {
        Alloc<complex_t> alloc;
        std::vector<complex_t, Alloc<complex_t>> vec(alloc);
        assert_vector_values<complex_t>(vec);
    }
#pragma endregion
}