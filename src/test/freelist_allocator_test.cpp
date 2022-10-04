#include "test.h"

#include "shared/allocation_utility.h"

#include "ktl/allocators/freelist_allocator.h"

namespace ktl
{
    KTL_ADD_TEST(test_freelist_allocator)
    {
        constexpr double value1 = 42.5;
        constexpr double value2 = 81.3;
        constexpr double value3 = 384.6;
        constexpr double value4 = 182.1;
        constexpr double value5 = 99.9;

        freelist_allocator<double> alloc;

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

    KTL_ADD_TEST(test_freelist_allocator_packed)
    {
        // Has to be bigger than 16, but not aligned
#pragma pack(push, 1)
        struct packed_t
        {
            int* ptr1;
            int* ptr2;
            uint16_t shorty;
            char caca;

            bool operator==(const packed_t& rhs) const
            {
                return ptr1 == rhs.ptr1 && ptr2 == rhs.ptr2 && shorty == rhs.shorty && caca == rhs.caca;
            }
        };
#pragma pack(pop)

        int forRef1 = 42;
        int forRef2 = 9;

        packed_t value1 = { &forRef1, &forRef2, 55, 'm' };
        packed_t value2 = { &forRef1, &forRef2, 982, 'c' };
        packed_t value3 = { &forRef1, &forRef2, 761, 'p' };
        packed_t value4 = { &forRef1, &forRef2, 666, 'd' };
        packed_t value5 = { &forRef1, &forRef2, 40000, 'a' };

        freelist_allocator<packed_t> alloc;

        packed_t* ptr1 = assert_allocate<packed_t>(alloc, value1);
        packed_t* ptr2 = assert_allocate<packed_t>(alloc, value2);
        packed_t* ptr3 = assert_allocate<packed_t>(alloc, value3);

        KTL_ASSERT(*ptr1 == value1);
        KTL_ASSERT(*ptr2 == value2);
        KTL_ASSERT(*ptr3 == value3);

        assert_deallocate<packed_t>(alloc, ptr1);

        packed_t* ptr4 = assert_allocate<packed_t>(alloc, value4);
        packed_t* ptr5 = assert_allocate<packed_t>(alloc, value5);

        KTL_ASSERT(*ptr4 == value4);
        KTL_ASSERT(*ptr5 == value5);

        assert_deallocate<packed_t>(alloc, ptr2);
        assert_deallocate<packed_t>(alloc, ptr4);
        assert_deallocate<packed_t>(alloc, ptr3);
        assert_deallocate<packed_t>(alloc, ptr5);
    }
}