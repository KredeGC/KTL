#include "shared/assert_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/stack_allocator.h"

#include "ktl/ptrs/ref_ptr.h"
#include "ktl/ptrs/weakref_ptr.h"

// Naming scheme: test_ref_ptr_[Type]
// Contains tests that relate directly to the ktl::ref_ptr

namespace ktl::test
{
    KTL_ADD_TEST(test_ref_ptr_make_std_double)
    {
        ref_ptr<double> ptr(42.0);

        KTL_TEST_ASSERT(*ptr == 42.0);
    }

    KTL_ADD_TEST(test_ref_ptr_copy_stack_double)
    {
        using alloc_t = type_stack_allocator<double, 128>;

        stack<128> block;
        alloc_t alloc(block);

        ref_ptr<double, alloc_t> ptr1(42.0, alloc);
        ref_ptr<double, alloc_t> ptr2(ptr1);

        KTL_TEST_ASSERT(*ptr1 == 42.0);
        KTL_TEST_ASSERT(*ptr2 == 42.0);
    }

    KTL_ADD_TEST(test_ref_ptr_copy_stack_complex)
    {
        using alloc_t = type_stack_allocator<complex_t, 128>;

        stack<128> block;
        alloc_t alloc(block);

        ref_ptr<complex_t, alloc_t> ptr1(42.0, alloc);
        ref_ptr<complex_t, alloc_t> ptr2(ptr1);

        KTL_TEST_ASSERT(*ptr1 == 42.0);
        KTL_TEST_ASSERT(*ptr2 == 42.0);
    }

    KTL_ADD_TEST(test_ref_ptr_move_stack_complex)
    {
        using alloc_t = type_stack_allocator<complex_t, 128>;

        stack<128> block;
        alloc_t alloc(block);

        ref_ptr<complex_t, alloc_t> ptr1(42.0, alloc);
        ref_ptr<complex_t, alloc_t> ptr2(std::move(ptr1));

        KTL_TEST_ASSERT(!ptr1);
        KTL_TEST_ASSERT(*ptr2 == 42.0);
    }

    KTL_ADD_TEST(test_weakref_ptr_stack_complex)
    {
        using alloc_t = type_stack_allocator<complex_t, 128>;

        stack<128> block;
        alloc_t alloc(block);

        ref_ptr<complex_t, alloc_t> ptr(42.0, alloc);

        weakref_ptr<complex_t, alloc_t> weak_ptr(ptr);

        KTL_TEST_ASSERT(*ptr == 42.0);

        ptr.reset();

        KTL_TEST_ASSERT(!weak_ptr);
        KTL_TEST_ASSERT(weak_ptr.expired());
    }
}