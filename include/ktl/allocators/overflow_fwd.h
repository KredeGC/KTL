#pragma once

#include <cstddef>
#include <ostream>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
	class type_allocator;
    
    // notomic
    template<typename T>
    struct notomic;
    
    // overflow
	template<typename Alloc, std::ostream& Stream, typename Atomic = notomic<size_t>>
	class overflow;

	template<typename T, typename A, std::ostream& Stream, typename Atomic = notomic<size_t>>
	using type_overflow_allocator = type_allocator<T, overflow<A, Stream, Atomic>>;
}