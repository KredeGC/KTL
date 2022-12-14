#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

#include <vector>

namespace ktl::performance::std_vector
{
    template<typename Alloc>
    void run_benchmark(const Alloc& alloc)
    {
        std::vector<trivial_t, Alloc> vec(alloc);

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }

    KTL_ADD_BENCHMARK(std_vector_push_std_allocator_trivial)
    {
        profiler::pause();

        run_benchmark(std::allocator<trivial_t>());
    }

    KTL_ADD_BENCHMARK(std_vector_push_stack_allocator_trivial)
    {
        profiler::pause();

        auto block = new stack<65536>;
        type_stack_allocator<trivial_t, 65536> alloc(block);

        run_benchmark(alloc);

        delete block;
    }
}