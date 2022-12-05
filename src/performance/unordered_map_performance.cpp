#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/random.h"

#include "ktl/containers/unordered_probe_map.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

namespace ktl::performance::unordered_map
{
    template<typename Alloc>
    void insert_benchmark(const Alloc& alloc)
    {
        ktl::unordered_probe_map<std::string, trivial_t, std::hash<std::string>, std::equal_to<std::string>, Alloc> map(alloc);

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            map.insert(std::to_string(i), trivial_t{ 42.0, 58.0 });

        profiler::pause();
    }

    template<typename Alloc>
    void lookup_benchmark(const Alloc& alloc)
    {
        ktl::unordered_probe_map<size_t, trivial_t, std::hash<size_t>, std::equal_to<size_t>, Alloc> map(alloc);
        map.reserve(1000);

        for (size_t i = 0; i < 1000; i++)
            map.insert(i, trivial_t{ 42.0, 58.0 });

        profiler::resume();

        trivial_t value;
        for (size_t i = 0; i < 1000; i++)
            value = map.find(i)->second;

        volatile double vol = value.gCost;

        profiler::pause();
    }

    template<typename Alloc>
    void range_benchmark(const Alloc& alloc)
    {
        ktl::unordered_probe_map<std::string, trivial_t, std::hash<std::string>, std::equal_to<std::string>, Alloc> map(alloc);
        map.reserve(1000);

        for (size_t i = 0; i < 1000; i++)
            map.insert(std::to_string(i), trivial_t{ 42.0, 58.0 });

        profiler::resume();

        trivial_t value;
        for (auto& kv : map)
            value = kv.second;

        volatile double vol = value.gCost;

        profiler::pause();
    }

    KTL_ADD_BENCHMARK(unordered_map_insert_std_allocator)
    {
        profiler::pause();

        insert_benchmark(std::allocator<std::pair<const std::string, trivial_t>>());
    }

    KTL_ADD_BENCHMARK(unordered_map_insert_stack_allocator)
    {
        profiler::pause();

        auto block = new stack<262144>;
        type_stack_allocator<std::pair<const std::string, trivial_t>, 262144> alloc(ktl::stack_allocator<262144>{ block });

        insert_benchmark(alloc);

        delete block;
    }

    KTL_ADD_BENCHMARK(unordered_map_lookup_std_allocator)
    {
        profiler::pause();

        lookup_benchmark(std::allocator<std::pair<const size_t, trivial_t>>());
    }

    KTL_ADD_BENCHMARK(unordered_map_range_std_allocator)
    {
        profiler::pause();

        range_benchmark(std::allocator<std::pair<const std::string, trivial_t>>());
    }
}