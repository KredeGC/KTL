#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_fwd.h"

#include <memory>
#include <type_traits>

#define KTL_PERFORMANCE_RUN(perform, type) profiler::pause(); { \
    std::allocator<type> alloc; \
    profiler::resume(); \
    perform<type, 1000>(alloc); }

namespace ktl::performance
{
    KTL_ADD_PERFORMANCE(std_allocator_init)
    {
        std::allocator<trivial_t> alloc;

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(std_allocator_uninit)
    {
        profiler::pause();

        {
            std::allocator<trivial_t> alloc;

            profiler::resume();
        }
    }

    KTL_ADD_PERFORMANCE(std_allocator_allocate_trivial)
    {
        KTL_PERFORMANCE_RUN(perform_allocation, trivial_t);
    }

    KTL_ADD_PERFORMANCE(std_allocator_deallocate_ordered_trivial)
    {
        KTL_PERFORMANCE_RUN(perform_ordered_deallocation, trivial_t);
    }

    KTL_ADD_PERFORMANCE(std_allocator_deallocate_unordered_trivial)
    {
        KTL_PERFORMANCE_RUN(perform_unordered_deallocation, trivial_t);
    }
}