#include "binary_heap_test.h"
#include "assert_utility.h"
#include "ktl/binary_heap.h"

#include "ktl/debug_allocator.h"
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
    void binary_heap_test()
    {
        node_t nodeTmp{ -1.0f, -1.0f };

        binary_min_heap<node_t, freelist_allocator<node_t>> heap(3);
        heap.insert({ 0.0f, 0.0f, { &nodeTmp } });
        heap.insert({ 28.0f, 24.0f });
        heap.insert({ 58.0f, 31.0f });
        heap.insert({ 32.0f, 8.0f });
        heap.insert({ 10.0f, 9.0f });
        heap.insert({ 9.0f, 11.0f });
        heap.insert({ 20.0f, 3.0f });
        heap.insert({ 8.0f, 7.0f });

        size_t size = heap.size();
        std::vector<node_t> nodes;

        /*for (size_t i = 0; i < size; i++)
           nodes.push_back(heap.pop());

        KSL_ASSERT(nodes.size() == size);*/

        for (size_t i = 0; i < size; i++)
           std::cout << heap.pop() << std::endl;
    }
}