#pragma once

#include "assert_utility.h"

namespace ktl
{
    template<typename T, typename Alloc>
    static T* assert_allocate(Alloc& alloc, const T& value)
    {
        T* ptr = std::allocator_traits<Alloc>::allocate(alloc, 1);

        if (ptr)
        {
            std::allocator_traits<Alloc>::construct(alloc, ptr, value);

            KTL_ASSERT(*ptr == value);
        }
        else
        {
            KTL_ASSERT_FALSE();
        }

        return ptr;
    }

    template<typename T, typename Alloc>
    void assert_deallocate(Alloc& alloc, T* ptr)
    {
        if (ptr)
        {
            std::allocator_traits<Alloc>::destroy(alloc, ptr);

            std::allocator_traits<Alloc>::deallocate(alloc, ptr, 1);
        }
        else
        {
            KTL_ASSERT_FALSE();
        }
    }

    template<typename Alloc, typename T>
    void assert_allocate_unordered(Alloc& alloc, const T* values)
    {
        T* ptr1 = assert_allocate(alloc, values[0]);
        T* ptr2 = assert_allocate(alloc, values[1]);
        T* ptr3 = assert_allocate(alloc, values[2]);

        KTL_ASSERT(*ptr1 == values[0]);
        KTL_ASSERT(*ptr2 == values[1]);
        KTL_ASSERT(*ptr3 == values[2]);

        assert_deallocate(alloc, ptr1);

        T* ptr4 = assert_allocate(alloc, values[3]);
        T* ptr5 = assert_allocate(alloc, values[4]);

        KTL_ASSERT(*ptr4 == values[3]);
        KTL_ASSERT(*ptr5 == values[4]);

        assert_deallocate(alloc, ptr4);
        assert_deallocate(alloc, ptr2);
        assert_deallocate(alloc, ptr3);
        assert_deallocate(alloc, ptr5);
    }
}