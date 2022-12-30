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
    
    template<typename P, typename F>
    struct fallback_builder<P, F>
    {
        using type = typename ktl::fallback<P, F>;
    };
    
    template<typename P, typename ...Args>
    struct fallback_builder<P, Args...>
    {
        using type = typename ktl::fallback<P, typename fallback_builder<Args...>::type>;
    };
    
    template<typename ...Args>
    using fallback_builder_t = typename fallback_builder<Args...>::type;
}