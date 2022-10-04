#include "test.h"

#include "shared/assert_utility.h"
#include "shared/types.h"

#include "ktl/containers/binary_heap.h"

#include "ktl/allocators/freelist_allocator.h"
#include "ktl/allocators/stack_allocator.h"

#include <algorithm>
#include <ostream>
#include <random>
#include <vector>

namespace ktl
{
    std::random_device rd;
    std::mt19937 random_generator(rd());

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

    KTL_ADD_TEST(test_binary_heap_double)
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

        binary_min_heap<double, freelist_allocator<double>> min_heap(3);
        test_binary_heap_insert_pop(min_heap, values, size);

        std::sort(values, values + size, std::greater<double>());
        binary_max_heap<double, freelist_allocator<double>> max_heap(3);
        test_binary_heap_insert_pop(max_heap, values, size);
    }

    KTL_ADD_TEST(test_binary_heap_trivial)
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

        binary_min_heap<trivial_t, freelist_allocator<trivial_t>> min_heap(3);
        test_binary_heap_insert_pop(min_heap, values, size);

        std::sort(values, values + size, std::greater<trivial_t>());
        binary_max_heap<trivial_t, freelist_allocator<trivial_t>> max_heap(3);
        test_binary_heap_insert_pop(max_heap, values, size);
    }

    KTL_ADD_TEST(test_binary_heap_complex)
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

        binary_min_heap<complex_t, freelist_allocator<complex_t>> min_heap(3);
        test_binary_heap_insert_pop(min_heap, values, size);

        std::sort(values, values + size, std::greater<complex_t>());
        binary_max_heap<complex_t, freelist_allocator<complex_t>> max_heap(3);
        test_binary_heap_insert_pop(max_heap, values, size);
    }
}