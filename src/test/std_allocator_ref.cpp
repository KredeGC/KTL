#include "test/shared/allocation_utility.h"
#include "test/shared/profiler.h"

#include <memory>
#include <type_traits>

namespace ktl
{
#pragma region std::allocator Performance
    KTL_ADD_PERFORMANCE(performance_std_allocator_ordered_double)
    {
        std::allocator<double> alloc;

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
        {
            double* ptr = alloc.allocate(1);
            alloc.deallocate(ptr, 1);
        }
    }

    KTL_ADD_PERFORMANCE(performance_std_allocator_unordered_double)
    {
        std::allocator<double> alloc;

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<double>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_std_allocator_unordered_trivial)
    {
        std::allocator<trivial_t> alloc;

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<trivial_t>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_std_allocator_unordered_packed)
    {
        std::allocator<packed_t> alloc;

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<packed_t>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_std_allocator_unordered_complex)
    {
        std::allocator<complex_t> alloc;

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<complex_t>(alloc);
    }
#pragma endregion
}