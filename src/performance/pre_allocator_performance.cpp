#include "shared/allocation_utility.h"
#include "shared/profiler.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/pre_allocator.h"

#define KTL_PERFORMANCE_RUN(perform, type) profiler::pause(); \
    auto block = new arena<sizeof(type) * 2000>; \
    type_pre_allocator<type, sizeof(type) * 2000> alloc(block); \
    profiler::resume(); \
    perform<type, 1000>(alloc); \
    profiler::pause();

namespace ktl::performance
{
    KTL_ADD_PERFORMANCE(pre_allocator_init)
    {
        auto block = new arena<16384>;
        type_pre_allocator<double, 16384> alloc(*block);

        profiler::pause();

        delete block;
    }

    KTL_ADD_PERFORMANCE(pre_allocator_uninit)
    {
        profiler::pause();

        {
            auto block = new arena<16384>;
            type_pre_allocator<double, 16384> alloc(block);

            delete block;

            profiler::resume();
        }
    }

    KTL_ADD_PERFORMANCE(pre_allocator_allocate_ordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_ordered_allocation, double);
    }

    KTL_ADD_PERFORMANCE(pre_allocator_deallocate_ordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_ordered_deallocation, double);
    }

    KTL_ADD_PERFORMANCE(pre_allocator_allocate_unordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_unordered_allocation, double);
    }

    KTL_ADD_PERFORMANCE(pre_allocator_deallocate_unordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_unordered_deallocation, double);
    }
}