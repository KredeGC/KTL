#pragma once
#include "assert_utility.h"
#include "random.h"
#include "types.h"

#include "ktl/containers/binary_heap.h"

namespace ktl::test
{
    template<typename Heap, typename T>
    void assert_binary_heap_insert_pop(Heap& heap, const T* values, size_t amount)
    {
        T* random_copy = new T[amount];
        std::copy(values, values + amount, random_copy);

        std::shuffle(random_copy, random_copy + amount, random_generator);

        for (size_t i = 0; i < amount; i++)
            heap.insert(random_copy[i]);

        for (size_t i = 0; i < amount; i++)
            KTL_TEST_ASSERT(heap.pop() == values[i]);

        delete[] random_copy;
    }

    template<typename Alloc, typename T>
    void assert_binary_heap_min_max(size_t heap_size, const Alloc& alloc, T* values, size_t amount)
    {
        // Use within scopes to ensure the allocator is destoyed
        {
            binary_min_heap<T, Alloc> min_heap(heap_size, alloc);
            assert_binary_heap_insert_pop(min_heap, values, amount);
        }

        // Reuse block once the previous allocator is done with it
        // Only matters for complex types with destructors
        {
            std::sort(values, values + amount, std::greater<T>());
            binary_max_heap<T, Alloc> max_heap(heap_size, alloc);
            assert_binary_heap_insert_pop(max_heap, values, amount);
        }
    }

    template<typename T, typename Alloc>
    typename std::enable_if<std::is_same<T, double>::value, void>::type
    assert_binary_heap(size_t heap_size, const Alloc& alloc)
    {
        constexpr size_t size = 8;

        double values[] = {
            0.0,
            8.0,
            9.0,
            10.0,
            20.0,
            28.0,
            32.0,
            58.0
        };

        assert_binary_heap_min_max(heap_size, alloc, values, size);
    }

    template<typename T, typename Alloc>
    typename std::enable_if<std::is_same<T, trivial_t>::value, void>::type
    assert_binary_heap(size_t heap_size, const Alloc& alloc)
    {
        constexpr size_t size = 8;

        trivial_t values[] = {
            { 0.0f, 0.0f },
            { 8.0f, 7.0f },
            { 10.0f, 9.0f },
            { 9.0f, 11.0f },
            { 20.0f, 3.0f },
            { 32.0f, 8.0f },
            { 28.0f, 24.0f },
            { 58.0f, 31.0f }
        };

        assert_binary_heap_min_max(heap_size, alloc, values, size);
    }

    template<typename T, typename Alloc>
    typename std::enable_if<std::is_same<T, packed_t>::value, void>::type
    assert_binary_heap(size_t heap_size, const Alloc& alloc)
    {
        constexpr size_t size = 8;

        int forRef1 = 42;
        int forRef2 = 9;

        packed_t values[] = {
            { &forRef1, &forRef2, 1, 'q' },
            { &forRef1, &forRef2, 55, 'm' },
            { &forRef1, &forRef2, 300, 'p' },
            { &forRef1, &forRef2, 666, 'd' },
            { &forRef1, &forRef2, 761, 'p' },
            { &forRef1, &forRef2, 888, 'n' },
            { &forRef1, &forRef2, 982, 'c' },
            { &forRef1, &forRef2, 40000, 'a' }
        };

        assert_binary_heap_min_max(heap_size, alloc, values, size);
    }

    template<typename T, typename Alloc>
    typename std::enable_if<std::is_same<T, complex_t>::value, void>::type
    assert_binary_heap(size_t heap_size, const Alloc& alloc)
    {
        constexpr size_t size = 8;

        complex_t values[size] = {
            0.0,
            8.0,
            9.0,
            10.0,
            20.0,
            28.0,
            32.0,
            58.0
        };

        assert_binary_heap_min_max(heap_size, alloc, values, size);
    }
}