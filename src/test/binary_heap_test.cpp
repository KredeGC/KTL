#include "shared/assert_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/containers/binary_heap.h"

#include "ktl/allocators/freelist_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_binary_heap_[Alloc]_[Type]
// Contains tests that use the ktl::binary_heap container, both min and max

namespace ktl
{
    KTL_ADD_TEST(test_binary_heap_std_double)
    {
        std::allocator<double> alloc;
        assert_binary_heap<double>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_freelist_double)
    {
        freelist<4096> block;
        type_freelist_allocator<double, 4096> alloc(block);
        assert_binary_heap<double>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_freelist_trivial)
    {
        freelist<4096> block;
        type_freelist_allocator<trivial_t, 4096> alloc(block);
        assert_binary_heap<trivial_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_freelist_packed)
    {
        freelist<4096> block;
        type_freelist_allocator<packed_t, 4096> alloc(block);
        assert_binary_heap<packed_t>(3, alloc);
    }

    KTL_ADD_TEST(test_binary_heap_freelist_complex)
    {
        freelist<4096> block;
        type_freelist_allocator<complex_t, 4096> alloc(block);
        assert_binary_heap<complex_t>(3, alloc);
    }
}