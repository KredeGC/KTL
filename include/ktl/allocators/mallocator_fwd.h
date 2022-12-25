#pragma once

#include <cstddef>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
    class type_allocator;
    
	// mallocator
	class mallocator;

	template<typename T>
	using type_mallocator = type_allocator<T, mallocator>;
}