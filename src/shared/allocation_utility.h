#pragma once

#include "assert_utility.h"
#include "random.h"
#include "types.h"

namespace ktl::test
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
    void assert_allocate_deallocate(Alloc& alloc, const T* values)
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

    template<typename T, typename Alloc>
    typename std::enable_if<std::is_same<T, double>::value, void>::type
    assert_unordered_values(Alloc& alloc)
    {
        double values[] = {
            42.5,
            81.3,
            384.6,
            182.1,
            99.9
        };

        assert_allocate_deallocate(alloc, values);
    }

    template<typename T, typename Alloc>
    typename std::enable_if<std::is_same<T, trivial_t>::value, void>::type
    assert_unordered_values(Alloc& alloc)
    {
        trivial_t values[] = {
            { 0.0f, 0.0f },
            { 8.0f, 7.0f },
            { 10.0f, 9.0f },
            { 9.0f, 11.0f },
            { 20.0f, 3.0f }
        };

        assert_allocate_deallocate(alloc, values);
    }

    template<typename T, typename Alloc>
    typename std::enable_if<std::is_same<T, packed_t>::value, void>::type
    assert_unordered_values(Alloc& alloc)
    {
        int forRef1 = 42;
        int forRef2 = 9;

        packed_t values[] = {
            { &forRef1, &forRef2, 55, 'm' },
            { &forRef1, &forRef2, 982, 'c' },
            { &forRef1, &forRef2, 761, 'p' },
            { &forRef1, &forRef2, 666, 'd' },
            { &forRef1, &forRef2, 40000, 'a' }
        };

        assert_allocate_deallocate(alloc, values);
    }

    template<typename T, typename Alloc>
    typename std::enable_if<std::is_same<T, complex_t>::value, void>::type
    assert_unordered_values(Alloc& alloc)
    {
        complex_t values[] = {
            42.5,
            81.3,
            384.6,
            182.1,
            99.9
        };

        assert_allocate_deallocate(alloc, values);
    }
}