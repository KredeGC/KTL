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
        template<typename, typename, typename>
        struct tuple_split_indices;

        template<size_t... N, size_t... M, typename... T>
        struct tuple_split_indices<std::index_sequence<N...>, std::index_sequence<M...>, std::tuple<T...>>
        {
            // The middle point is at sizeof...(N)
            using first = std::tuple<typename std::tuple_element_t<N, std::tuple<T...>>...>;

            // Offset by N + 1, since we don't include the middle type
            using second = std::tuple<typename std::tuple_element_t<M + 1 + sizeof...(N), std::tuple<T...>>...>;
        };


        // Helper struct for inverting sizes
        template<bool, size_t, size_t>
        struct size_invert;

        template<size_t N, size_t M>
        struct size_invert<false, N, M>
        {
            static constexpr size_t first = N;
            static constexpr size_t second = M - 1 - N;
        };

        template<size_t N, size_t M>
        struct size_invert<true, N, M>
        {
            static constexpr size_t first = M - 1 - N;
            static constexpr size_t second = N;
        };


        // Recursive helper struct for generating the segragator type
        template<bool, typename...>
        struct segragator_builder;

        // When a single type remains, just return it
        template<bool R, typename Alloc>
        struct segragator_builder<R, std::tuple<Alloc>>
        {
            using type = Alloc;
        };

        // Takes in a tuple of types and returns the final segragator
        template<bool R, typename... Ts>
        struct segragator_builder<R, std::tuple<Ts...>>
        {
            static_assert(sizeof...(Ts) % 2 != 0, "The number of allocators needs to match the number of thresholds + 1");

            // The middle of the parameter pack is the root of a complete binary tree
            // The root of a complete binary tree is (2^(log2(N)) - 1) / 2
            using middle = size_invert<R, (pow2<sizeof...(Ts)>::Result - 1) / 2, sizeof...(Ts)>;

            // Split the parameter pack into 2 tuples
            using split = tuple_split_indices<
                std::make_index_sequence<middle::first>, // Up to, but not including the middle threshold
                std::make_index_sequence<middle::second>, // Anything after the middle threshold
                std::tuple<Ts...>>;

            // This points to the root of a complete binary tree
            using threshold = typename std::tuple_element_t<middle::first, std::tuple<Ts...>>;

            // Create this struct recursively for both sides of the middle point
            using first = typename segragator_builder<R, typename split::first>::type;
            using second = typename segragator_builder<R, typename split::second>::type;

            using type = segragator<threshold::value, first, second>;
        };
    }

    // Helper type without the tuple
    template<typename ...Ts>
    using segragator_builder_l = typename detail::segragator_builder<false, std::tuple<Ts...>>::type;

    template<typename ...Ts>
    using segragator_builder_r = typename detail::segragator_builder<true, std::tuple<Ts...>>::type;

    template<size_t N>
    using threshold = std::integral_constant<size_t, N>;
}