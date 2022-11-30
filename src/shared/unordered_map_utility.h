#pragma once
#include "assert_utility.h"
#include "types.h"

namespace ktl::test
{
    template<typename Map, typename K, typename V>
    void assert_unordered_map_insert(Map& map, const K* keys, const V* values, size_t amount)
    {
        for (size_t i = 0; i < amount; i++)
            map.insert(keys[i], values[i]);

        KTL_TEST_ASSERT(map.size() == amount);
        KTL_TEST_ASSERT(map.capacity() >= 3);

        // Assert operator[]
        for (size_t i = 0; i < amount; i++)
            KTL_TEST_ASSERT(map[keys[i]] == values[i]);

        // Assert iterators
        for (auto& [key, value] : map)
        {
            bool exists = false;
            for (size_t i = 0; i < amount; i++)
            {
                if (key == keys[i] && value == values[i])
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
                KTL_TEST_ASSERT_FALSE();
        }

        // Erase each element, 1 by 1
        for (size_t i = 0; i < amount; i++)
        {
            auto iter = map.find(keys[i]);

            if (iter != map.end())
            {
                map.erase(iter);

                KTL_TEST_ASSERT(map.size() == (amount - i - 1));
                KTL_TEST_ASSERT(map.find(keys[i]) == map.end());
            }
            else
            {
                KTL_TEST_ASSERT_FALSE();
            }
        }
    }

    template<typename T, typename Map>
    typename std::enable_if<std::is_same<T, double>::value, void>::type
    assert_unordered_map_values(Map& map)
    {
        constexpr size_t size = 8;

        std::string keys[] = {
            "Test1",
            "Something",
            "A very long string to ensure no small-string optimization",
            "Otherwise yes",
            "15 byte for GCC",
            "A bit longer string",
            "More to it",
            "!!?+)(=)&/"
        };

        double values[] = {
            0.0,
            8.0,
            9.0,
            10.0,
            20.0,
            28.0,
            32.0,
            58.0
        };

        assert_unordered_map_insert(map, keys, values, size);
    }

    template<typename T, typename Map>
    typename std::enable_if<std::is_same<T, trivial_t>::value, void>::type
    assert_unordered_map_values(Map& map)
    {
        constexpr size_t size = 8;

        std::string keys[] = {
            "Test1",
            "Something",
            "A very long string to ensure no small-string optimization",
            "Otherwise yes",
            "15 byte for GCC",
            "A bit longer string",
            "More to it",
            "!!?+)(=)&/"
        };

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

        assert_unordered_map_insert(map, keys, values, size);
    }

    template<typename T, typename Map>
    typename std::enable_if<std::is_same<T, packed_t>::value, void>::type
    assert_unordered_map_values(Map& map)
    {
        constexpr size_t size = 8;

        std::string keys[] = {
            "Test1",
            "Something",
            "A very long string to ensure no small-string optimization",
            "Otherwise yes",
            "15 byte for GCC",
            "A bit longer string",
            "More to it",
            "!!?+)(=)&/"
        };

        int forRef1 = 42;
        int forRef2 = 9;

        packed_t values[] = {
            { &forRef1, &forRef2, 1, 'q' },
            { &forRef1, &forRef2, 55, 'm' },
            { &forRef1, &forRef2, 300, 'p' },
            { &forRef1, &forRef2, 666, 'd' },
            { &forRef1, &forRef2, 761, 'p' },
            { &forRef1, &forRef2, 888, 'n' },
            { &forRef1, &forRef2, 982, 'c' },
            { &forRef1, &forRef2, 40000, 'a' }
        };

        assert_unordered_map_insert(map, keys, values, size);
    }

    template<typename T, typename Map>
    typename std::enable_if<std::is_same<T, complex_t>::value, void>::type
    assert_unordered_map_values(Map& map)
    {
        constexpr size_t size = 8;

        std::string keys[] = {
            "Test1",
            "Something",
            "A very long string to ensure no small-string optimization",
            "Otherwise yes",
            "15 byte for GCC",
            "A bit longer string",
            "More to it",
            "!!?+)(=)&/"
        };

        complex_t values[size] = {
            0.0,
            8.0,
            9.0,
            10.0,
            20.0,
            28.0,
            32.0,
            58.0
        };

        assert_unordered_map_insert(map, keys, values, size);
    }
}