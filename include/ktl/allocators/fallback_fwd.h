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
    
    namespace detail
    {
        // Recursive helper struct for generating the fallback type
        template<typename ...Ts>
        struct fallback_builder;

        template<typename Primary, typename Fallback>
        struct fallback_builder<Primary, Fallback>
        {
            using type = fallback<Primary, Fallback>;
        };

        template<typename Alloc, typename ...Ts>
        struct fallback_builder<Alloc, Ts...>
        {
            using type = fallback<Alloc, typename fallback_builder<Ts...>::type>;
        };
    }
    
    template<typename ...Ts>
    using fallback_builder_t = typename detail::fallback_builder<Ts...>::type;
}