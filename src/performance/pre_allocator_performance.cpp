#include "shared/profiler.h"
#include "shared/types.h"

#include "ktl/allocators/pre_allocator.h"

namespace ktl::performance::pre_allocator
{
    template<size_t Size>
    using AllocType = type_pre_allocator<trivial_t, sizeof(trivial_t) * Size>;

    template<typename T, size_t Size, typename Func>
    void run_benchmark(Func func)
    {
        profiler::pause();

        AllocType<Size> alloc;

        func(alloc);
    }

    KTL_ADD_BENCHMARK(pre_allocator_init)
    {
        type_pre_allocator<trivial_t, 16384> alloc;

        profiler::pause();
    }

    KTL_ADD_BENCHMARK(pre_allocator_uninit)
    {
        profiler::pause();

        {
            type_pre_allocator<trivial_t, 16384> alloc;

            profiler::resume();
        }
    }

    KTL_ADD_BENCHMARK(pre_allocator_allocate_ordered_trivial)
    {
        constexpr size_t SIZE = 1000;
        run_benchmark<trivial_t, SIZE>(perform_allocation<trivial_t, SIZE, AllocType<SIZE>>);
    }

    KTL_ADD_BENCHMARK(pre_allocator_deallocate_ordered_trivial)
    {
        constexpr size_t SIZE = 1000;
        run_benchmark<trivial_t, SIZE>(perform_ordered_deallocation<trivial_t, SIZE, AllocType<SIZE>>);
    }

    KTL_ADD_BENCHMARK(pre_allocator_allocate_unordered_trivial)
    {
        constexpr size_t SIZE = 2000;
        run_benchmark<trivial_t, SIZE>(perform_unordered_allocation<trivial_t, SIZE, AllocType<SIZE>>);
    }

    KTL_ADD_BENCHMARK(pre_allocator_deallocate_unordered_trivial)
    {
        constexpr size_t SIZE = 1000;
        run_benchmark<trivial_t, SIZE>(perform_unordered_deallocation<trivial_t, SIZE, AllocType<SIZE>>);
    }
}