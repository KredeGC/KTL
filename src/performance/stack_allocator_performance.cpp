#include "shared/profiler.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/allocators/stack_allocator.h"

namespace ktl::performance
{
    typedef type_stack_allocator<trivial_t, sizeof(trivial_t) * 1000> AllocType;

    template<typename T, typename Func>
    void run_benchmark(Func func)
    {
        profiler::pause();

        auto block = new stack<sizeof(T) * 1000>;
        AllocType alloc(block);

        func(alloc);

        delete block;
    }

    KTL_ADD_BENCHMARK(stack_allocator_init)
    {
        auto block = new stack<16384>;
        type_stack_allocator<trivial_t, 16384> alloc(block);

        profiler::pause();

        delete block;
    }

    KTL_ADD_BENCHMARK(stack_allocator_uninit)
    {
        profiler::pause();

        {
            auto block = new stack<16384>;
            type_stack_allocator<trivial_t, 16384> alloc(block);

            delete block;

            profiler::resume();
        }
    }

    KTL_ADD_BENCHMARK(stack_allocator_allocate_trivial)
    {
        run_benchmark<trivial_t>(perform_allocation<trivial_t, 1000, AllocType>);
    }

    KTL_ADD_BENCHMARK(stack_allocator_deallocate_ordered_trivial)
    {
        run_benchmark<trivial_t>(perform_ordered_deallocation<trivial_t, 1000, AllocType>);
    }

    KTL_ADD_BENCHMARK(stack_allocator_deallocate_unordered_trivial)
    {
        run_benchmark<trivial_t>(perform_unordered_deallocation<trivial_t, 1000, AllocType>);
    }
}