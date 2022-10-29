#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_fwd.h"

#include "ktl/containers/trivial_vector.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

namespace ktl::performance
{
    KTL_ADD_PERFORMANCE(trivial_vector_std_allocator_double)
    {
        trivial_vector<double> vec;

        for (size_t i = 0; i < 1000; i++)
            vec.push_back(42.0);
    }

    KTL_ADD_PERFORMANCE(trivial_vector_mallocator_double)
    {
        trivial_vector<double, type_mallocator<double>> vec;

        for (size_t i = 0; i < 1000; i++)
            vec.push_back(42.0);
    }

    KTL_ADD_PERFORMANCE(trivial_vector_pre_allocator_double)
    {
        // TODO: Fix segfault in GCC release mode
        // Seems to be a problem with pre_allocator, that somehow doesn't affect normal allocations
        trivial_vector<double, type_pre_allocator<double, 16384>> vec;

        for (size_t i = 0; i < 500; i++)
            vec.push_back(42.0);
    }

    KTL_ADD_PERFORMANCE(trivial_vector_stack_allocator_double)
    {
        auto block = new stack<16384>;
        trivial_vector<double, type_stack_allocator<double, 16384>> vec({ block });

        for (size_t i = 0; i < 1000; i++)
            vec.push_back(42.0);
    }
}