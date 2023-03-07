#include "shared/profiler.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/shared.h"
#include "ktl/allocators/type_allocator.h"

namespace ktl::performance::linear_allocator
{
    typedef type_linear_allocator<trivial_t, sizeof(trivial_t) * 1000> AllocType;

    template<typename T, typename Func>
    void run_benchmark(Func func)
    {
        profiler::pause();

        AllocType alloc;

        func(alloc);
    }

    KTL_ADD_BENCHMARK(linear_allocator_init)
    {
        auto alloc = new type_linear_allocator<trivial_t, 16384>;

        profiler::pause();

        delete alloc;
    }

    KTL_ADD_BENCHMARK(linear_allocator_uninit)
    {
        profiler::pause();

        {
            auto alloc = new type_linear_allocator<trivial_t, 16384>;

            profiler::resume();

            delete alloc;
        }
    }

    KTL_ADD_BENCHMARK(linear_allocator_allocate_trivial)
    {
        run_benchmark<trivial_t>(perform_allocation<trivial_t, 1000, AllocType>);
    }

    KTL_ADD_BENCHMARK(linear_allocator_deallocate_ordered_trivial)
    {
        run_benchmark<trivial_t>(perform_ordered_deallocation<trivial_t, 1000, AllocType>);
    }

    KTL_ADD_BENCHMARK(linear_allocator_deallocate_unordered_trivial)
    {
        run_benchmark<trivial_t>(perform_unordered_deallocation<trivial_t, 1000, AllocType>);
    }
}