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
    
    // cascading
	template<typename Alloc, typename Atomic = notomic<size_t>>
	class cascading;

	template<typename T, typename Alloc, typename Atomic = notomic<size_t>>
	using type_cascading_allocator = type_allocator<T, cascading<Alloc, Atomic>>;
}