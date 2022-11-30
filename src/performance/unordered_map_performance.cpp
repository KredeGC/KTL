#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/random.h"

#include "ktl/containers/unordered_map.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

namespace ktl::performance
{
    template<typename Alloc>
    void run_benchmark(const Alloc& alloc)
    {
        ktl::unordered_map<std::string, trivial_t, std::hash<std::string>, std::equal_to<std::string>, Alloc> map(alloc);

        profiler::resume();

        for (size_t i = 0; i < 600; i++)
            map.insert(std::to_string(i), { 42.0, 58.0 });

        profiler::pause();
    }

    KTL_ADD_BENCHMARK(probe_unordered_map_insert_std_allocator)
    {
        profiler::pause();

        run_benchmark(std::allocator<std::pair<const std::string, trivial_t>>());
    }

    /*KTL_ADD_BENCHMARK(probe_unordered_map_insert_mallocator)
    {
        profiler::pause();

        run_benchmark(type_mallocator<std::pair<const std::string, trivial_t>>());
    }

    KTL_ADD_BENCHMARK(probe_unordered_map_insert_pre_allocator)
    {
        profiler::pause();

        run_benchmark(type_pre_allocator<std::pair<const std::string, trivial_t>, 65536>());
    }

    KTL_ADD_BENCHMARK(probe_unordered_map_insert_stack_allocator)
    {
        profiler::pause();

        auto block = new stack<65536>;
        type_stack_allocator<std::pair<const std::string, trivial_t>, 65536> alloc(block);

        run_benchmark(alloc);

        delete block;
    }*/
}