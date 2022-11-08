#pragma once

#include <iostream>

namespace ktl
{
	// stack
	template<size_t Size>
	struct stack;

	// type_allocator
	template<typename T, typename Alloc>
	class type_allocator;

	// cascading_allocator
	template<typename Alloc>
	class cascading_allocator;

	template<typename T, typename Alloc>
	using type_cascading_allocator = type_allocator<T, cascading_allocator<Alloc>>;

	// composite_allocator
	template<typename P, typename F>
	class composite_allocator;

	template<typename T, typename P, typename F>
	using type_composite_allocator = type_allocator<T, composite_allocator<P, F>>;

	// freelist_allocator
	template<size_t Min, size_t Max, typename Alloc>
	class freelist_allocator;

	template<typename T, size_t Min, size_t Max, typename A>
	using type_freelist_allocator = type_allocator<T, freelist_allocator<Min, Max, A>>;
	
	// mallocator
	class mallocator;

	template<typename T>
	using type_mallocator = type_allocator<T, mallocator>;

	// null_allocator
	class null_allocator;

	// overflow_allocator
	template<typename Alloc, std::ostream& Stream>
	class overflow_allocator;

	template<typename T, typename A, std::ostream& Stream>
	using type_overflow_allocator = type_allocator<T, overflow_allocator<A, Stream>>;

	// pre_allocator
	template<size_t Size>
	class pre_allocator;

	template<typename T, size_t Size>
	using type_pre_allocator = type_allocator<T, pre_allocator<Size>>;

	// segragator_allocator
	template<size_t Threshold, typename P, typename F>
	class segragator_allocator;

	template<typename T, size_t Threshold, typename P, typename F>
	using type_segragator_allocator = type_allocator<T, segragator_allocator<Threshold, P, F>>;

	// stack_allocator
	template<size_t Size>
	class stack_allocator;

	template<typename T, size_t Size>
	using type_stack_allocator = type_allocator<T, stack_allocator<Size>>;
}