#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/mallocator.h"

namespace ktl
{
    KTL_ADD_TEST(test_mallocator)
    {
        constexpr double value1 = 42.5;
        constexpr double value2 = 81.3;
        constexpr double value3 = 384.6;
        constexpr double value4 = 182.1;
        constexpr double value5 = 99.9;

        type_mallocator<double> alloc;

        double* ptr1 = assert_allocate<double>(alloc, value1);
        double* ptr2 = assert_allocate<double>(alloc, value2);
        double* ptr3 = assert_allocate<double>(alloc, value3);

        KTL_ASSERT(*ptr1 == value1);
        KTL_ASSERT(*ptr2 == value2);
        KTL_ASSERT(*ptr3 == value3);

        assert_deallocate<double>(alloc, ptr1);

        double* ptr4 = assert_allocate<double>(alloc, value4);
        double* ptr5 = assert_allocate<double>(alloc, value5);

        KTL_ASSERT(*ptr4 == value4);
        KTL_ASSERT(*ptr5 == value5);

        assert_deallocate<double>(alloc, ptr2);
        assert_deallocate<double>(alloc, ptr4);
        assert_deallocate<double>(alloc, ptr3);
        assert_deallocate<double>(alloc, ptr5);
    }
}