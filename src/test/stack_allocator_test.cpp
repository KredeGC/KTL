#include "shared/allocation_utility.h"
#include "shared/profiler.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_stack_allocator_[Type]
// Contains tests that relate directly to the ktl::stack_allocator

namespace ktl
{
    KTL_ADD_HEADER();

    KTL_ADD_TEST(test_stack_allocator_double)
    {
        stack<4096> block;
        type_stack_allocator<double, 4096> alloc(block);
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_stack_allocator_packed)
    {
        stack<4096> block;
        type_stack_allocator<packed_t, 4096> alloc(block);
        assert_unordered_values<packed_t>(alloc);
    }

#pragma region Performance
    KTL_ADD_PERFORMANCE(performance_stack_allocator_unordered_double)
    {
        stack<4096> block;
        type_stack_allocator<double, 4096> alloc(block);

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<double>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_stack_allocator_unordered_trivial)
    {
        stack<4096> block;
        type_stack_allocator<trivial_t, 4096> alloc(block);

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<trivial_t>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_stack_allocator_unordered_packed)
    {
        stack<4096> block;
        type_stack_allocator<packed_t, 4096> alloc(block);

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<packed_t>(alloc);
    }

    KTL_ADD_PERFORMANCE(performance_stack_allocator_unordered_complex)
    {
        stack<4096> block;
        type_stack_allocator<complex_t, 4096> alloc(block);

        for (size_t i = 0; i < profiler::NUM_ALLOCATIONS; i++)
            assert_unordered_values<complex_t>(alloc);
    }
#pragma endregion
}