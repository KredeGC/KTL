#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/containers/sbo_vector.h"
#include "ktl/containers/trivial_vector.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

namespace ktl::performance::sbo_vector
{
    void run_benchmark()
    {
        ktl::sbo_vector<trivial_vector<trivial_t>> vec{};

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            vec.push_back({ 42.0, 58.0 });

        profiler::pause();
    }

    KTL_ADD_BENCHMARK(sbo_vector_push_std_allocator_trivial)
    {
        profiler::pause();

        run_benchmark();
    }
}