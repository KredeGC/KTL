#include "shared/assert_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/stack_allocator.h"

#include "ktl/ptrs/ref_ptr.h"

// Naming scheme: test_ref_ptr_[Type]
// Contains tests that relate directly to the ktl::ref_ptr

namespace ktl::test
{
    KTL_ADD_TEST(test_ref_ptr_double)
    {
        ref_ptr<double> ptr = make_ref<double>(42.0);

        KTL_ASSERT(*ptr == 42.0);
    }

    KTL_ADD_TEST(test_ref_ptr_stack_double)
    {
        stack<128> block;
        type_stack_allocator<double, 128> alloc(block);

        ref_ptr<double, type_stack_allocator<double, 128>> ptr = allocate_ref<double>(alloc, 42.0);

        KTL_ASSERT(*ptr == 42.0);
    }

    KTL_ADD_TEST(test_ref_ptr_complex)
    {
        stack<128> block;
        type_stack_allocator<complex_t, 128> alloc(block);

        ref_ptr<complex_t, type_stack_allocator<complex_t, 128>> ptr = allocate_ref<complex_t>(alloc, 42.0);

        KTL_ASSERT(*ptr == 42.0);
    }
}