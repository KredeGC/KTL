#include "shared/allocation_utility.h"
#include "shared/profiler.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/stack_allocator.h"

#define KTL_PERFORMANCE_RUN(perform, type) profiler::pause(); { \
    auto block = new stack<sizeof(type) * 1000>; \
    type_stack_allocator<type, sizeof(type) * 1000> alloc(block); \
    profiler::resume(); \
    perform<type, 1000>(alloc); }

namespace ktl::performance
{
    KTL_ADD_PERFORMANCE(stack_allocator_init)
    {
        auto block = new stack<16384>;
        type_stack_allocator<double, 16384> alloc(block);

        profiler::pause();

        delete block;
    }

    KTL_ADD_PERFORMANCE(stack_allocator_uninit)
    {
        profiler::pause();

        {
            auto block = new stack<16384>;
            type_stack_allocator<double, 16384> alloc(block);

            delete block;

            profiler::resume();
        }
    }

    KTL_ADD_PERFORMANCE(stack_allocator_allocate_double)
    {
        KTL_PERFORMANCE_RUN(perform_allocation, double);
    }

    KTL_ADD_PERFORMANCE(stack_allocator_deallocate_ordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_ordered_deallocation, double);
    }

    KTL_ADD_PERFORMANCE(stack_allocator_deallocate_unordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_unordered_deallocation, double);
    }
}