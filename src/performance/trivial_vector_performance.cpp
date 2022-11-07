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
    KTL_ADD_PERFORMANCE(trivial_vector_push_std_allocator_trivial)
    {
        profiler::pause();

        trivial_vector<trivial_t> vec;

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(trivial_vector_push_mallocator_trivial)
    {
        profiler::pause();

        trivial_vector<trivial_t, type_mallocator<trivial_t>> vec;

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(trivial_vector_push_pre_allocator_trivial)
    {
        profiler::pause();

        trivial_vector<trivial_t, type_pre_allocator<trivial_t, 65536>> vec;

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(trivial_vector_push_stack_allocator_trivial)
    {
        profiler::pause();

        auto block = new stack<65536>;
        trivial_vector<trivial_t, type_stack_allocator<trivial_t, 65536>> vec({ block });

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }
}