#include "shared/allocation_utility.h"
#include "shared/profiler.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/mallocator.h"

#include <vector>

// Naming scheme: test_mallocator_[Container]_[Type]
// Contains tests that relate directly to the ktl::mallocator

namespace ktl
{
    KTL_ADD_HEADER();

    KTL_ADD_TEST(test_mallocator_double)
    {
        type_mallocator<double> alloc;
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_mallocator_packed)
    {
        type_mallocator<packed_t> alloc;
        assert_unordered_values<packed_t>(alloc);
    }

#pragma region std::vector
    KTL_ADD_TEST(test_mallocator_std_vector_double)
    {
        std::vector<double, type_mallocator<double>> vec;
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_mallocator_std_vector_trivial)
    {
        std::vector<trivial_t, type_mallocator<trivial_t>> vec;
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_mallocator_std_vector_packed)
    {
        std::vector<packed_t, type_mallocator<packed_t>> vec;
        assert_vector_values<packed_t>(vec);
    }

    KTL_ADD_TEST(test_mallocator_std_vector_complex)
    {
        std::vector<complex_t, type_mallocator<complex_t>> vec;
        assert_vector_values<complex_t>(vec);
    }
#pragma endregion

#pragma region Performance
    KTL_ADD_PERFORMANCE(performance_mallocator_unordered_double)
    {
        type_mallocator<double> alloc;

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<double>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_mallocator_unordered_trivial)
    {
        type_mallocator<trivial_t> alloc;

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<trivial_t>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_mallocator_unordered_packed)
    {
        type_mallocator<packed_t> alloc;

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<packed_t>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_mallocator_unordered_complex)
    {
        type_mallocator<complex_t> alloc;

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<complex_t>(alloc);
    }
#pragma endregion
}