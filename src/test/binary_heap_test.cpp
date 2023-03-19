#include "shared/assert_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/binary_heap.h"

#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/shared.h"
#include "ktl/allocators/stack_allocator.h"
#include "ktl/allocators/type_allocator.h"

// Naming scheme: test_binary_heap_[Alloc]_[Type]
// Contains tests that use the ktl::binary_heap container, both min and max

namespace ktl::test::binary_heap
{
    KTL_ADD_TEST(test_binary_heap_array_construct)
    {
        using Alloc = ktl::type_shared_linear_allocator<double, 2048>;
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
        assert_binary_heap_min_max<double, std::allocator<double>>();
    }

    KTL_ADD_TEST(test_binary_heap_std_trivial)
    {
        assert_binary_heap_min_max<trivial_t, std::allocator<trivial_t>>();
    }

    KTL_ADD_TEST(test_binary_heap_std_packed)
    {
        assert_binary_heap_min_max<packed_t, std::allocator<packed_t>>();
    }

    KTL_ADD_TEST(test_binary_heap_std_complex)
    {
        assert_binary_heap_min_max<complex_t, std::allocator<complex_t>>();
    }
#pragma endregion

#pragma region ktl::mallocator
    KTL_ADD_TEST(test_binary_heap_mallocator_double)
    {
        assert_binary_heap_min_max<double, type_mallocator<double>>();
    }

    KTL_ADD_TEST(test_binary_heap_mallocator_trivial)
    {
        assert_binary_heap_min_max<trivial_t, type_mallocator<trivial_t>>();
    }

    KTL_ADD_TEST(test_binary_heap_mallocator_packed)
    {
        assert_binary_heap_min_max<packed_t, type_mallocator<packed_t>>();
    }

    KTL_ADD_TEST(test_binary_heap_mallocator_complex)
    {
        assert_binary_heap_min_max<complex_t, type_mallocator<complex_t>>();
    }
#pragma endregion

#pragma region ktl::linear
    KTL_ADD_TEST(test_binary_heap_linear_double)
    {
        assert_binary_heap_min_max<double, type_linear_allocator<double, 4096>>();
    }

    KTL_ADD_TEST(test_binary_heap_linear_trivial)
    {
        assert_binary_heap_min_max<trivial_t, type_linear_allocator<trivial_t, 4096>>();
    }

    KTL_ADD_TEST(test_binary_heap_linear_packed)
    {
        assert_binary_heap_min_max<packed_t, type_linear_allocator<packed_t, 4096>>();
    }

    KTL_ADD_TEST(test_binary_heap_linear_complex)
    {
        assert_binary_heap_min_max<complex_t, type_linear_allocator<complex_t, 4096>>();
    }
#pragma endregion

#pragma region ktl::stack_allocator
    KTL_ADD_TEST(test_binary_heap_stack_double)
    {
        stack<4096> block;
        assert_binary_heap_min_max<double, type_stack_allocator<double, 4096>>(block);
    }

    KTL_ADD_TEST(test_binary_heap_stack_trivial)
    {
        stack<4096> block;
        assert_binary_heap_min_max<trivial_t, type_stack_allocator<trivial_t, 4096>>(block);
    }

    KTL_ADD_TEST(test_binary_heap_stack_packed)
    {
        stack<4096> block;
        assert_binary_heap_min_max<packed_t, type_stack_allocator<packed_t, 4096>>(block);
    }

    KTL_ADD_TEST(test_binary_heap_stack_complex)
    {
        stack<4096> block;
        assert_binary_heap_min_max<complex_t, type_stack_allocator<complex_t, 4096>>(block);
    }
#pragma endregion
}