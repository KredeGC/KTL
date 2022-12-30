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
    
    template<typename P, typename Threshold, typename F>
    struct segragator_builder<P, Threshold, F>
    {
        using type = typename ktl::segragator<Threshold::value, P, F>;
    };
    
    template<typename P, typename Threshold, typename ...Args>
    struct segragator_builder<P, Threshold, Args...>
    {
        using type = typename ktl::segragator<Threshold::value, P, typename segragator_builder<Args...>::type>;
    };
    
    template<typename ...Args>
    using segragator_builder_t = typename segragator_builder<Args...>::type;
    
    template<size_t N>
    using threshold = std::integral_constant<size_t, N>;
}