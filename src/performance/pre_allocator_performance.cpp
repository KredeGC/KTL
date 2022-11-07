#include "shared/profiler.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/pre_allocator.h"

#define KTL_PERFORMANCE_RUN(perform, type, amount) profiler::pause(); { \
    type_pre_allocator<type, sizeof(type) * amount> alloc; \
    profiler::resume(); \
    perform<type, amount>(alloc); }

namespace ktl::performance
{
    KTL_ADD_PERFORMANCE(pre_allocator_init)
    {
        type_pre_allocator<trivial_t, 16384> alloc;

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(pre_allocator_uninit)
    {
        profiler::pause();

        {
            type_pre_allocator<trivial_t, 16384> alloc;

            profiler::resume();
        }
    }

    KTL_ADD_PERFORMANCE(pre_allocator_allocate_trivial)
    {
        KTL_PERFORMANCE_RUN(perform_allocation, trivial_t, 1000);
    }

    KTL_ADD_PERFORMANCE(pre_allocator_deallocate_ordered_trivial)
    {
        KTL_PERFORMANCE_RUN(perform_ordered_deallocation, trivial_t, 1000);
    }

    KTL_ADD_PERFORMANCE(pre_allocator_allocate_unordered_trivial)
    {
        KTL_PERFORMANCE_RUN(perform_unordered_allocation, trivial_t, 2000);
    }

    KTL_ADD_PERFORMANCE(pre_allocator_deallocate_unordered_trivial)
    {
        KTL_PERFORMANCE_RUN(perform_unordered_deallocation, trivial_t, 1000);
    }
}