#pragma once

#include <cstddef>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
	class type_allocator;
    
    // notomic
    template<typename T>
    struct notomic;
    
	// freelist
	template<size_t Min, size_t Max, typename Alloc, typename Atomic = notomic<size_t>>
	class freelist;

	template<typename T, size_t Min, size_t Max, typename Alloc, typename Atomic = notomic<size_t>>
	using type_freelist_allocator = type_allocator<T, freelist<Min, Max, Alloc, Atomic>>;
}