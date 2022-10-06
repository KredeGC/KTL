#include "shared/allocation_utility.h"
#include "shared/profiler.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/pre_allocator.h"

// Naming scheme: test_pre_allocator_[Type]
// Contains tests that relate directly to the ktl::pre_allocator

namespace ktl
{
    KTL_ADD_HEADER();

    KTL_ADD_TEST(test_freelist_allocator_double)
    {
        arena<4096> block;
        type_pre_allocator<double, 4096> alloc(block);
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_freelist_allocator_packed)
    {
        arena<4096> block;
        type_pre_allocator<packed_t, 4096> alloc(block);
        assert_unordered_values<packed_t>(alloc);
    }

#pragma region Performance
    KTL_ADD_PERFORMANCE(performance_pre_allocator_ordered_double)
    {
        arena<4096> block;
        type_pre_allocator<double, 4096> alloc(block);

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
        {
            double* ptr = alloc.allocate(1);
            alloc.deallocate(ptr, 1);
        }
    }

    KTL_ADD_PERFORMANCE(performance_pre_allocator_unordered_double)
    {
        arena<4096> block;
        type_pre_allocator<double, 4096> alloc(block);

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<double>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_pre_allocator_unordered_trivial)
    {
        arena<4096> block;
        type_pre_allocator<trivial_t, 4096> alloc(block);

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<trivial_t>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_pre_allocator_unordered_packed)
    {
        arena<4096> block;
        type_pre_allocator<packed_t, 4096> alloc(block);

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<packed_t>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_pre_allocator_unordered_complex)
    {
        arena<4096> block;
        type_pre_allocator<complex_t, 4096> alloc(block);

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<complex_t>(alloc);
    }
#pragma endregion
}