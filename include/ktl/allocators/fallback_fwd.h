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
}