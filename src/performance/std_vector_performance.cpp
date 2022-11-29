#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

#include <vector>

namespace ktl::performance
{
    KTL_ADD_PERFORMANCE(std_vector_push_std_allocator_trivial)
    {
        profiler::pause();

        std::vector<trivial_t> vec;

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(std_vector_push_mallocator_trivial)
    {
        profiler::pause();

        std::vector<trivial_t, type_mallocator<trivial_t>> vec;

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(std_vector_push_pre_allocator_trivial)
    {
        profiler::pause();

        std::vector<trivial_t, type_pre_allocator<trivial_t, 65536>> vec;

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }

    KTL_ADD_PERFORMANCE(std_vector_push_stack_allocator_trivial)
    {
        profiler::pause();

        auto block = new stack<65536>;
        type_stack_allocator<trivial_t, 65536> alloc(block);
        std::vector<trivial_t, type_stack_allocator<trivial_t, 65536>> vec(alloc);

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }
}