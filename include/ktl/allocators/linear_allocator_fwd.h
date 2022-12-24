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
    
    // linear_allocator
    template<size_t Size, typename Atomic = notomic<size_t>>
	class linear_allocator;

	template<typename T, size_t Size, typename Atomic = notomic<size_t>>
	using type_linear_allocator = type_allocator<T, linear_allocator<Size, Atomic>>;
}