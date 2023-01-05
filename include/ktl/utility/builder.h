#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

namespace ktl::detail
{
    // Helper struct for getting the closest power of 2
    template<size_t N>
    struct pow2
    {
        enum : size_t
        {
            A = N - 1,
            B = A | (A >> 1),
            C = B | (B >> 2),
            D = C | (C >> 4),
            E = D | (D >> 8),
            F = E | (E >> 16),
            G = F | (F >> 32),
            Result = G + 1
        };
    };


    // Helper struct for splitting a tuple based on indices
    template<size_t, typename, typename, typename>
    struct tuple_split_indices;

    template<size_t O, size_t... N, size_t... M, typename... T>
    struct tuple_split_indices<O, std::index_sequence<N...>, std::index_sequence<M...>, std::tuple<T...>>
    {
        // The middle point is at sizeof...(N)
        using first = std::tuple<typename std::tuple_element_t<N, std::tuple<T...>>...>;

        // Offset by N + O, since we don't include the middle type
        using second = std::tuple<typename std::tuple_element_t<M + O + sizeof...(N), std::tuple<T...>>...>;
    };


    // Helper struct for inverting sizes
    template<bool, size_t, size_t>
    struct size_invert;

    template<size_t N, size_t M>
    struct size_invert<false, N, M>
    {
        static constexpr size_t first = N;
        static constexpr size_t second = M - N;
    };

    template<size_t N, size_t M>
    struct size_invert<true, N, M>
    {
        static constexpr size_t first = M - N;
        static constexpr size_t second = N;
    };
}