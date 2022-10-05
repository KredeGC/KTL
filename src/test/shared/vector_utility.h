#pragma once
#include "assert_utility.h"
#include "types.h"

#include "ktl/containers/trivial_vector.h"

namespace ktl
{
    template<typename Vec, typename T>
    void assert_vector_push_back(Vec& vec, const T* values, size_t amount)
    {
        for (size_t i = 0; i < amount; i++)
            vec.push_back(values[i]);

        for (size_t i = 0; i < amount; i++)
            KTL_ASSERT(vec[i] == values[i]);
    }

    template<typename T, typename Vec>
    typename std::enable_if<std::is_same<T, double>::value, void>::type
    assert_vector_values(Vec& vector)
    {
        constexpr size_t size = 8;

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

        assert_vector_push_back(vector, values, size);
    }

    template<typename T, typename Vec>
    typename std::enable_if<std::is_same<T, trivial_t>::value, void>::type
        assert_vector_values(Vec& vector)
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

        assert_vector_push_back(vector, values, size);
    }
}