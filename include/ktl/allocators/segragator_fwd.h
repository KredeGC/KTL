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

    // Helper function for getting the closest power of 2
    constexpr size_t pow2_ceil(size_t v)
    {
        --v;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        return ++v;
    }

    // Helper struct for splitting a tuple in 2
    template<typename, typename, typename>
    struct tuple_split;

    template<std::size_t... N, std::size_t... M, typename T>
    struct tuple_split<std::index_sequence<N...>, std::index_sequence<M...>, T>
    {
        // The middle point is at sizeof...(N)
        using first = std::tuple<typename std::tuple_element_t<N, T>...>;

        // Offset by N + 1, since we don't want the middle point in either
        using second = std::tuple<typename std::tuple_element_t<M + 1 + sizeof...(N), T>...>;
    };

    // Recursive helper struct for generating the segragator type
    template<typename... Ts>
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
    template<class... Ts>
    struct segragator_builder<std::tuple<Ts...>>
    {
        static_assert(sizeof...(Ts) % 2 != 0, "The number of allocators needs to match the number of thresholds + 1");

        // The middle of the type is the root of a complete binary tree
        // The root of a complete binary tree is (2^(log2(N)) - 1) / 2
        static constexpr size_t max = sizeof...(Ts);
        static constexpr size_t middle = (pow2_ceil(max) - 1) / 2;

        using split = tuple_split<
            std::make_index_sequence<middle>, // Up to, but not including the middle threshold
            std::make_index_sequence<max - middle - 1>, // Anything after the middle threshold
            std::tuple<Ts...>>;

        using threshold = typename std::tuple_element_t<middle, std::tuple<Ts...>>;
        using first = typename segragator_builder<typename split::first>::type;
        using second = typename segragator_builder<typename split::second>::type;

        using type = segragator<threshold::value, first, second>;
    };

    // Helper type without the tuple
    template<typename ...Ts>
    using segragator_builder_t = typename segragator_builder<std::tuple<Ts...>>::type;

    template<size_t N>
    using threshold = std::integral_constant<size_t, N>;
}