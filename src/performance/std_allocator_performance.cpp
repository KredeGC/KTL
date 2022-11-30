#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include <memory>
#include <type_traits>

namespace ktl::performance::std_allocator
{
    typedef std::allocator<trivial_t> AllocType;

    template<typename T, typename Func>
    void run_benchmark(Func func)
    {
        profiler::pause();

        AllocType alloc;

        func(alloc);
    }

    KTL_ADD_BENCHMARK(std_allocator_init)
    {
        std::allocator<trivial_t> alloc;

        profiler::pause();
    }

    KTL_ADD_BENCHMARK(std_allocator_uninit)
    {
        profiler::pause();

        {
            std::allocator<trivial_t> alloc;

            profiler::resume();
        }
    }

    KTL_ADD_BENCHMARK(std_allocator_allocate_trivial)
    {
        run_benchmark<trivial_t>(perform_allocation<trivial_t, 1000, AllocType>);
    }

    KTL_ADD_BENCHMARK(std_allocator_deallocate_ordered_trivial)
    {
        run_benchmark<trivial_t>(perform_ordered_deallocation<trivial_t, 1000, AllocType>);
    }

    KTL_ADD_BENCHMARK(std_allocator_deallocate_unordered_trivial)
    {
        run_benchmark<trivial_t>(perform_unordered_deallocation<trivial_t, 1000, AllocType>);
    }
}