#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/mallocator.h"

#define KTL_PERFORMANCE_RUN(perform, type) profiler::pause(); { \
    type_mallocator<type> alloc; \
    profiler::resume(); \
    perform<type, 1000>(alloc); }

namespace ktl::performance
{
    KTL_ADD_PERFORMANCE(mallocator_init)
    {
        type_mallocator<double> alloc;

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(mallocator_uninit)
    {
        profiler::pause();

        {
            type_mallocator<double> alloc;

            profiler::resume();
        }
    }

    KTL_ADD_PERFORMANCE(mallocator_allocate_double)
    {
        KTL_PERFORMANCE_RUN(perform_allocation, double);
    }

    KTL_ADD_PERFORMANCE(mallocator_deallocate_ordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_ordered_deallocation, double);
    }

    KTL_ADD_PERFORMANCE(mallocator_deallocate_unordered_double)
    {
        KTL_PERFORMANCE_RUN(perform_unordered_deallocation, double);
    }
}