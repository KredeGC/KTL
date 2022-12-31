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
        using type = typename ktl::segragator<Threshold::value, Primary, Fallback>;
    };
    
    template<typename Alloc, typename Threshold, typename ...Args>
    struct segragator_builder<Alloc, Threshold, Args...>
    {
        using type = typename ktl::segragator<Threshold::value, Alloc, typename segragator_builder<Args...>::type>;
    };
    
    template<typename ...Args>
    using segragator_builder_t = typename segragator_builder<Args...>::type;
    
    template<size_t N>
    using threshold = std::integral_constant<size_t, N>;
}