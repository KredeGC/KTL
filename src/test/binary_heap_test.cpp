#include "shared/assert_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/binary_heap.h"

#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/linked.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_binary_heap_[Alloc]_[Type]
// Contains tests that use the ktl::binary_heap container, both min and max

namespace ktl::test::binary_heap
{
    KTL_ADD_TEST(test_binary_heap_array_construct)
    {
        using Alloc = ktl::type_linear_allocator<double, 2048>;
        using Container = ktl::binary_min_heap<double, Alloc>;

        constexpr size_t size = 4;

        double values[] = {
            4.0,
            8.0,
            -1.0,
            10.0
        };

        Container baseContainer;

        Alloc allocator;

        assert_construct_container<Container>(
            [&](Container& lhs, Container& rhs)
        {
            // Comparison function
            KTL_TEST_ASSERT(lhs.peek() == rhs.peek());
            KTL_TEST_ASSERT(lhs.size() == rhs.size());
        }, [&]()
        {
            // Insert some elements
            for (size_t i = 0; i < size; i++)
                baseContainer.insert(values[i]);

            return baseContainer;
        }, [&]()
        {
            // Construct using initializer list
            Container container{ values[0], values[1], values[2], values[3] };

            KTL_TEST_ASSERT(!allocator.owns(container.begin()));

            return container;
        }, [&]()
        {
            // Construct by copying using a different allocator
            Container container(baseContainer, allocator);

            KTL_TEST_ASSERT(allocator.owns(container.begin()));

            return container;
        }, [&]()
        {
            // Construct by moving using a different allocator
            Container container(std::move(baseContainer), allocator);

            KTL_TEST_ASSERT(baseContainer.empty());
            KTL_TEST_ASSERT(allocator.owns(container.begin()));

            return container;
        });
    }

#pragma region std::allocator
    KTL_ADD_TEST(test_binary_heap_std_double)
    {
        std::allocator<double> alloc;
        assert_binary_heap<double>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_std_trivial)
    {
        std::allocator<trivial_t> alloc;
        assert_binary_heap<trivial_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_std_packed)
    {
        std::allocator<packed_t> alloc;
        assert_binary_heap<packed_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_std_complex)
    {
        std::allocator<complex_t> alloc;
        assert_binary_heap<complex_t>(3, alloc);
    }
#pragma endregion

#pragma region ktl::mallocator
    KTL_ADD_TEST(test_binary_heap_mallocator_double)
    {
        ktl::type_mallocator<double> alloc;
        assert_binary_heap<double>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_mallocator_trivial)
    {
        ktl::type_mallocator<trivial_t> alloc;
        assert_binary_heap<trivial_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_mallocator_packed)
    {
        ktl::type_mallocator<packed_t> alloc;
        assert_binary_heap<packed_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_mallocator_complex)
    {
        ktl::type_mallocator<complex_t> alloc;
        assert_binary_heap<complex_t>(3, alloc);
    }
#pragma endregion

#pragma region ktl::linked
    KTL_ADD_TEST(test_binary_heap_list_double)
    {
        type_linked_allocator<double, 4096, mallocator> alloc;
        assert_binary_heap<double>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_list_trivial)
    {
        type_linked_allocator<trivial_t, 4096, mallocator> alloc;
        assert_binary_heap<trivial_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_list_packed)
    {
        type_linked_allocator<packed_t, 4096, mallocator> alloc;
        assert_binary_heap<packed_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_list_complex)
    {
        type_linked_allocator<complex_t, 4096, mallocator> alloc;
        assert_binary_heap<complex_t>(3, alloc);
    }
#pragma endregion

#pragma region ktl::stack_allocator
    KTL_ADD_TEST(test_binary_heap_stack_double)
    {
        stack<4096> block;
        type_stack_allocator<double, 4096> alloc(block);
        assert_binary_heap<double>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_stack_trivial)
    {
        stack<4096> block;
        type_stack_allocator<trivial_t, 4096> alloc(block);
        assert_binary_heap<trivial_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_stack_packed)
    {
        stack<4096> block;
        type_stack_allocator<packed_t, 4096> alloc(block);
        assert_binary_heap<packed_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_stack_complex)
    {
        stack<4096> block;
        type_stack_allocator<complex_t, 4096> alloc(block);
        assert_binary_heap<complex_t>(3, alloc);
    }
#pragma endregion
}