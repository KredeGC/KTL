#include "shared/profiler.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/pre_allocator.h"

#define KTL_PERFORMANCE_RUN(perform, type, amount) profiler::pause(); { \
    type_pre_allocator<type, sizeof(type) * amount * 2> alloc; \
    profiler::resume(); \
    perform<type, amount>(alloc); }

namespace ktl::performance
{
    KTL_ADD_PERFORMANCE(pre_allocator_init)
    {
        type_pre_allocator<double, 16384> alloc;

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(pre_allocator_uninit)
    {
        profiler::pause();

        {
            type_pre_allocator<double, 16384> alloc;

            profiler::resume();
        }
    }

    KTL_ADD_PERFORMANCE(pre_allocator_allocate_double)
    {
        KTL_PERFORMANCE_RUN(perform_allocation, double, 1000);
    }

    KTL_ADD_PERFORMANCE(pre_allocator_deallocate_ordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_ordered_deallocation, double, 1000);
    }

    KTL_ADD_PERFORMANCE(pre_allocator_allocate_unordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_unordered_allocation, double, 2000);
    }

    KTL_ADD_PERFORMANCE(pre_allocator_deallocate_unordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_unordered_deallocation, double, 1000);
    }
}