#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
	class type_allocator;
    
    // segragator
	template<size_t Threshold, typename P, typename F>
	class segragator;

	template<typename T, size_t Threshold, typename P, typename F>
	using type_segragator_allocator = type_allocator<T, segragator<Threshold, P, F>>;

    namespace detail
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

            // Offset by N + O
            using second = std::tuple<typename std::tuple_element_t<M + O + sizeof...(N), std::tuple<T...>>...>;
        };

        // Helper struct for splitting a tuple based on a middle point
        template<size_t N, typename... Ts>
        struct tuple_split_reverse
            : tuple_split_indices<1,
                std::make_index_sequence<sizeof...(Ts) - 1 - N>, // Up to, but not including the middle threshold
                std::make_index_sequence<N>, // Anything after the middle threshold
                std::tuple<Ts...>>
        {
            // This points to the root of a right-leaning complete binary tree
            using threshold = typename std::tuple_element_t<sizeof...(Ts) - 1 - N, std::tuple<Ts...>>;
        };

        // Recursive helper struct for generating the segragator type
        template<typename...>
        struct segragator_builder;

        // When three types remain, use a segragator
        template<typename Primary, typename Threshold, typename Secondary>
        struct segragator_builder<std::tuple<Primary, Threshold, Secondary>>
        {
            using type = segragator<Threshold::value, Primary, Secondary>;
        };

        // When a single type remains, just return it
        template<typename Alloc>
        struct segragator_builder<std::tuple<Alloc>>
        {
            using type = Alloc;
        };

        // Takes in a tuple of types and returns the final segragator
        template<typename... Ts>
        struct segragator_builder<std::tuple<Ts...>>
        {
            static_assert(sizeof...(Ts) % 2 != 0, "The number of allocators needs to match the number of thresholds + 1");

            // The middle of the parameter pack is the root of a complete binary tree
            // The root of a complete binary tree is (2^(log2(N)) - 1) / 2
            using split = tuple_split_reverse<
                (pow2<sizeof...(Ts)>::Result - 1) / 2,
                Ts...>;

            // Create this struct recursively for both sides of the middle point
            using threshold = typename split::threshold;
            using first = typename segragator_builder<typename split::first>::type;
            using second = typename segragator_builder<typename split::second>::type;

            using type = segragator<threshold::value, first, second>;
        };
    }

    // Helper type without the tuple
    template<typename ...Ts>
    using segragator_builder_t = typename detail::segragator_builder<std::tuple<Ts...>>::type;

    template<size_t N>
    using threshold = std::integral_constant<size_t, N>;
}