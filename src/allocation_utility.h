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
}