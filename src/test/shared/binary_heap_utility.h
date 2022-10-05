#pragma once
#include "assert_utility.h"

#include <algorithm>
#include <random>

namespace ktl
{
    inline std::random_device rd;
    inline std::mt19937 random_generator(rd());

    template<typename Heap, typename T>
    void test_binary_heap_insert_pop(Heap& heap, const T* values, size_t amount)
    {
        T* random_copy = new T[amount];
        std::copy(values, values + amount, random_copy);

        std::shuffle(random_copy, random_copy + amount, random_generator);

        for (size_t i = 0; i < amount; i++)
            heap.insert(random_copy[i]);

        for (size_t i = 0; i < amount; i++)
            KTL_ASSERT(heap.pop() == values[i]);

        delete[] random_copy;
    }

    template<size_t HeapSize, typename Alloc, typename T>
    void test_binary_heap_min_max(const Alloc& alloc, T* values, size_t amount)
    {
        // Use within scopes to ensure the allocator is destoyed
        {
            binary_min_heap<T, Alloc> min_heap(HeapSize, alloc);
            test_binary_heap_insert_pop(min_heap, values, amount);
        }

        // Reuse block once the previous allocator is done with it
        // Only matters for complex types with destructors
        {
            std::sort(values, values + amount, std::greater<T>());
            binary_max_heap<T, Alloc> max_heap(HeapSize, alloc);
            test_binary_heap_insert_pop(max_heap, values, amount);
        }
    }
}