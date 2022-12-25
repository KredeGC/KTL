#include "shared/profiler.h"
#include "shared/types.h"
#include "shared/random.h"

#include "ktl/allocators/list_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

#include <unordered_map>

namespace ktl::performance::std_unordered_map
{
    template<typename Alloc>
    void run_benchmark(const Alloc& alloc)
    {
        std::unordered_map<std::string, trivial_t, std::hash<std::string>, std::equal_to<std::string>, Alloc> map(alloc);

        profiler::resume();

        for (size_t i = 0; i < 1000; i++)
            map.insert({ std::to_string(i), { 42.0, 58.0 } });

        profiler::pause();
    }

    template<typename Alloc>
    void lookup_benchmark(const Alloc& alloc)
    {
        std::unordered_map<size_t, trivial_t, std::hash<size_t>, std::equal_to<size_t>, Alloc> map(alloc);
        map.reserve(1000);

        for (size_t i = 0; i < 1000; i++)
            map.insert({ i, trivial_t{ 42.0, 58.0 } });

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
        std::unordered_map<std::string, trivial_t, std::hash<std::string>, std::equal_to<std::string>, Alloc> map(alloc);
        map.reserve(1000);

        for (size_t i = 0; i < 1000; i++)
            map.insert({ std::to_string(i), trivial_t{ 42.0, 58.0 } });

        profiler::resume();

        trivial_t value;
        for (auto& kv : map)
            value = kv.second;

        volatile double vol = value.gCost;

        profiler::pause();
    }

    KTL_ADD_BENCHMARK(std_unordered_map_insert_std_allocator)
    {
        profiler::pause();

        run_benchmark(std::allocator<std::pair<const std::string, trivial_t>>());
    }

    KTL_ADD_BENCHMARK(std_unordered_map_insert_stack_allocator)
    {
        profiler::pause();

        auto block = new stack<262144>;
        type_stack_allocator<std::pair<const std::string, trivial_t>, 262144> alloc(ktl::stack_allocator<262144>{ block });

        run_benchmark(alloc);

        delete block;
    }

    KTL_ADD_BENCHMARK(std_unordered_map_lookup_std_allocator)
    {
        profiler::pause();

        lookup_benchmark(std::allocator<std::pair<const size_t, trivial_t>>());
    }

    KTL_ADD_BENCHMARK(std_unordered_map_lookup_stack_allocator)
    {
        profiler::pause();

        auto block = new stack<262144>;
        type_stack_allocator<std::pair<const size_t, trivial_t>, 262144> alloc(ktl::stack_allocator<262144>{ block });

        lookup_benchmark(alloc);

        delete block;
    }

    KTL_ADD_BENCHMARK(std_unordered_map_range_std_allocator)
    {
        profiler::pause();

        range_benchmark(std::allocator<std::pair<const std::string, trivial_t>>());
    }

    KTL_ADD_BENCHMARK(std_unordered_map_range_stack_allocator)
    {
        profiler::pause();

        auto block = new stack<262144>;
        type_stack_allocator<std::pair<const std::string, trivial_t>, 262144> alloc(ktl::stack_allocator<262144>{ block });

        range_benchmark(alloc);

        delete block;
    }
}