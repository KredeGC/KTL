#pragma once

#include <iostream>

namespace ktl
{
	template<typename T, typename Alloc>
	class type_allocator;

	// composite_allocator
	template<typename P, typename F>
	class composite_allocator;

	template<typename T, typename P, typename F>
	using type_composite_allocator = type_allocator<T, composite_allocator<P, F>>;

	// freelist_allocator
	class freelist_allocator;

	template<typename T>
	using type_freelist_allocator = type_allocator<T, freelist_allocator>;
	
	// mallocator
	class mallocator;

	template<typename T>
	using type_mallocator = type_allocator<T, mallocator>;

	// overflow_allocator
	template<typename Alloc, std::ostream& Stream>
	class overflow_allocator;

	// stack_allocator
	class stack_allocator;

	template<typename T>
	using type_stack_allocator = type_allocator<T, stack_allocator>;
}