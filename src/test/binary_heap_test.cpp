#include "shared/assert_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/binary_heap.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_binary_heap_[Alloc]_[Type]
// Contains tests that use the ktl::binary_heap container, both min and max

namespace ktl::test
{
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

#pragma region ktl::freelist_allocator
    KTL_ADD_TEST(test_binary_heap_pre_double)
    {
        type_pre_allocator<double, 4096> alloc;
        assert_binary_heap<double>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_pre_trivial)
    {
        type_pre_allocator<trivial_t, 4096> alloc;
        assert_binary_heap<trivial_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_pre_packed)
    {
        type_pre_allocator<packed_t, 4096> alloc;
        assert_binary_heap<packed_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_pre_complex)
    {
        type_pre_allocator<complex_t, 4096> alloc;
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