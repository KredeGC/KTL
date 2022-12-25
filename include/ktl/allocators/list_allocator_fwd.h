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
    
    // list_allocator
	template<size_t Size, typename Atomic = notomic<size_t>>
	class list_allocator;

	template<typename T, size_t Size, typename Atomic = notomic<size_t>>
	using type_list_allocator = type_allocator<T, list_allocator<Size, Atomic>>;
}