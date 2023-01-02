#pragma once

#include <cstddef>

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
    
    // variadic builder
    template<typename ...Args>
    struct segragator_builder;
    
    template<typename Primary, typename Threshold, typename Fallback>
    struct segragator_builder<Primary, Threshold, Fallback>
    {
        using type = segragator<Threshold::value, Primary, Fallback>;
    };
    
    template<typename Alloc, typename Threshold, typename ...Args>
    struct segragator_builder<Alloc, Threshold, Args...>
    {
        using type = segragator<Threshold::value, Alloc, typename segragator_builder<Args...>::type>;
    };
    
    template<typename ...Args>
    using segragator_builder_t = typename segragator_builder<Args...>::type;
    
    template<size_t N>
    using threshold = std::integral_constant<size_t, N>;



    template <class, class, class>
    struct half_half;

    template <std::size_t... N, std::size_t... M, class T>
    struct half_half<std::index_sequence<N...>, std::index_sequence<M...>, T>
    {
        using first = std::tuple<typename std::tuple_element_t<N, T>...>;

        using second = std::tuple<typename std::tuple_element_t<M + 1 + sizeof...(N), T>...>;
    };


    template<class... Ts>
    struct tuple_half;

    // When there are only 3 types left
    template<typename Primary, typename Threshold, typename Secondary>
    struct tuple_half<std::tuple<Primary, Threshold, Secondary>>
    {
        using type = segragator<Threshold::value, Primary, Secondary>;
    };

    // Takes in a tuple of types
    template<class... Ts>
    struct tuple_half<std::tuple<Ts...>>
    {
        using threshold = typename std::tuple_element<sizeof...(Ts) / 2, std::tuple<Ts...>>::type;

        using split = half_half<
            std::make_index_sequence<sizeof...(Ts) / 2>,
            std::make_index_sequence<sizeof...(Ts) / 2>,
            std::tuple<Ts...>>;

        using first = typename tuple_half<typename split::first>::type;

        using second = typename tuple_half<typename split::second>::type;

        using type = segragator<threshold::value, first, second>;
    };

    template<class... Ts>
    struct tuple_full
    {
        using type = typename tuple_half<std::tuple<Ts...>>::type;
    };
}