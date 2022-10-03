#include "binary_heap_test.h"
#include "assert_utility.h"
#include "ktl/binary_heap.h"

#include "ktl/freelist_allocator.h"
#include "ktl/stack_allocator.h"

#include <iostream>
#include <ostream>
#include <vector>

struct node_t
{
    float gCost;
    float hCost;
    std::vector<node_t*> Neighbours;

    bool operator==(const node_t& other) const
    {
        return gCost == other.gCost
            && hCost == other.hCost
            && Neighbours.size() == other.Neighbours.size();
    }

    bool operator<(const node_t& other) const
    {
        return (gCost + hCost) < (other.gCost + other.hCost);
    }

    bool operator>(const node_t& other) const
    {
        return (gCost + hCost) > (other.gCost + other.hCost);
    }

    friend std::ostream& operator<<(std::ostream& output, const node_t& rhs)
    {
        return output << "[g:" << rhs.gCost << ", h:" << rhs.hCost << "]";
    }
};

namespace ktl
{
    void test_min_heap_complex()
    {
        node_t nodeTmp{ -1.0f, -1.0f };

        node_t values[] = {
            { 0.0f, 0.0f, { &nodeTmp } },
            { 8.0f, 7.0f },
            { 10.0f, 9.0f },
            { 9.0f, 11.0f },
            { 20.0f, 3.0f },
            { 32.0f, 8.0f },
            { 28.0f, 24.0f },
            { 58.0f, 31.0f }
        };

        binary_min_heap<node_t, freelist_allocator<node_t>> heap(3);
        heap.insert(values[0]);
        heap.insert(values[5]);
        heap.insert(values[7]);
        heap.insert(values[6]);
        heap.insert(values[3]);
        heap.insert(values[2]);
        heap.insert(values[4]);
        heap.insert(values[1]);

        size_t size = heap.size();
        for (size_t i = 0; i < size; i++)
            KTL_ASSERT(heap.pop() == values[i]);

        /*auto data = heap.data();
        for (size_t i = 0; i < size; i++)
            std::cout << data[i] << std::endl;

        for (size_t i = 0; i < size; i++)
            std::cout << heap.pop() << std::endl;*/
    }

    void test_min_heap_double()
    {
        constexpr double values[] = {
            0.0,
            8.0,
            9.0,
            10.0,
            20.0,
            28.0,
            32.0,
            58.0
        };

        binary_min_heap<double, freelist_allocator<double>> heap(3);
        heap.insert(values[0]);
        heap.insert(values[5]);
        heap.insert(values[7]);
        heap.insert(values[6]);
        heap.insert(values[3]);
        heap.insert(values[2]);
        heap.insert(values[4]);
        heap.insert(values[1]);

        size_t size = heap.size();
        for (size_t i = 0; i < size; i++)
            KTL_ASSERT(heap.pop() == values[i]);

        /*auto data = heap.data();
        for (size_t i = 0; i < size; i++)
            std::cout << data[i] << std::endl;

        for (size_t i = 0; i < size; i++)
            std::cout << heap.pop() << std::endl;*/
    }
}