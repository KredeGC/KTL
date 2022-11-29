#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/allocators/mallocator.h"

namespace ktl::performance
{
    typedef type_mallocator<trivial_t> AllocType;

    template<typename T, typename Func>
    void run_benchmark(Func func)
    {
        profiler::pause();

        AllocType alloc;

        func(alloc);
    }

    KTL_ADD_BENCHMARK(mallocator_init)
    {
        type_mallocator<trivial_t> alloc;

        profiler::pause();
    }

    KTL_ADD_BENCHMARK(mallocator_uninit)
    {
        profiler::pause();

        {
            type_mallocator<trivial_t> alloc;

            profiler::resume();
        }
    }

    KTL_ADD_BENCHMARK(mallocator_allocate_trivial)
    {
        run_benchmark<trivial_t>(perform_allocation<trivial_t, 1000, AllocType>);
    }

    KTL_ADD_BENCHMARK(mallocator_deallocate_ordered_trivial)
    {
        run_benchmark<trivial_t>(perform_ordered_deallocation<trivial_t, 1000, AllocType>);
    }

    KTL_ADD_BENCHMARK(mallocator_deallocate_unordered_trivial)
    {
        run_benchmark<trivial_t>(perform_unordered_deallocation<trivial_t, 1000, AllocType>);
    }
}