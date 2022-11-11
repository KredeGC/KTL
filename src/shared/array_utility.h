#pragma once
#include "assert_utility.h"
#include "random.h"
#include "types.h"

namespace ktl::test
{
	template<size_t Count, typename Array, typename T>
	void assert_array_insert(Array& arr, const T* values)
	{
        arr.resize(Count);

        for (size_t i = 0; i < Count; i++)
            arr[i] = values[i];

        // operator[] access
        for (size_t i = 0; i < Count; i++)
            KTL_ASSERT(arr[i] == values[i]);

        // Reverse loops from Size - 1
        size_t counter = Count;
        for (auto iter = arr.rbegin(); iter != arr.rend(); iter++)
            KTL_ASSERT(*iter == values[--counter]);

        // Normal loops from 0
        for (auto& value : arr)
            KTL_ASSERT(value == values[counter++]);
	}

	template<typename T, typename Array>
    typename std::enable_if<std::is_same<T, double>::value, void>::type
    assert_array_values(Array& arr)
    {
        constexpr size_t size = 8;

        constexpr double values[] = {
            0.0,
            8.0,
            9.0,
            10.0,
            20.0,
            28.0,
            32.0,
            58.0
        };

        assert_array_insert<size>(arr, values);
    }

	template<typename T, typename Array>
    typename std::enable_if<std::is_same<T, trivial_t>::value, void>::type
    assert_array_values(Array& arr)
    {
        constexpr size_t size = 8;

        trivial_t values[] = {
            { 0.0f, 0.0f },
            { 8.0f, 7.0f },
            { 10.0f, 9.0f },
            { 9.0f, 11.0f },
            { 20.0f, 3.0f },
            { 32.0f, 8.0f },
            { 28.0f, 24.0f },
            { 58.0f, 31.0f }
        };

        assert_array_insert<size>(arr, values);
    }

	template<typename T, typename Array>
    typename std::enable_if<std::is_same<T, packed_t>::value, void>::type
    assert_array_values(Array& arr)
    {
        constexpr size_t size = 8;

        int forRef1 = 42;
        int forRef2 = 9;

        packed_t values[] = {
            { &forRef1, &forRef2, 40000, 'a' },
            { &forRef2, &forRef2, 1, 'q' },
            { &forRef1, &forRef2, 300, 'p' },
            { &forRef2, &forRef1, 55, 'm' },
            { &forRef1, &forRef2, 888, 'n' },
            { &forRef2, &forRef1, 761, 'p' },
            { &forRef2, &forRef2, 666, 'd' },
            { &forRef1, &forRef1, 982, 'c' }
        };

        assert_array_insert<size>(arr, values);
    }
}