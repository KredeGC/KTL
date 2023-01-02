#pragma once

#include <cstddef>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
	class type_allocator;
    
	// fallback
	template<typename P, typename F>
	class fallback;

	template<typename T, typename P, typename F>
	using type_fallback_allocator = type_allocator<T, fallback<P, F>>;
    
    // variadic builder
    template<typename ...Args>
    struct fallback_builder;
    
    template<typename Primary, typename Fallback>
    struct fallback_builder<Primary, Fallback>
    {
        using type = typename fallback<Primary, Fallback>;
    };
    
    template<typename Alloc, typename ...Args>
    struct fallback_builder<Alloc, Args...>
    {
        using type = typename fallback<Alloc, typename fallback_builder<Args...>::type>;
    };
    
    template<typename ...Args>
    using fallback_builder_t = typename fallback_builder<Args...>::type;
}