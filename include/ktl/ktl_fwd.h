#pragma once

#include <iostream>

namespace ktl
{
	// stack
	template<size_t Size>
	struct stack;

	// arena
	template<size_t Size>
	struct arena;

	// type_allocator
	template<typename T, typename Alloc>
	class type_allocator;

	// composite_allocator
	template<typename P, typename F>
	class composite_allocator;

	template<typename T, typename P, typename F>
	using type_composite_allocator = type_allocator<T, composite_allocator<P, F>>;

	// freelist_allocator
	template<size_t Min, size_t Max, size_t Batch, size_t Threshold, typename Alloc>
	class freelist_allocator;

	template<typename T, size_t Min, size_t Max, size_t Batch, size_t Threshold, typename A>
	using type_freelist_allocator = type_allocator<T, freelist_allocator<Min, Max, Batch, Threshold, A>>;
	
	// mallocator
	class mallocator;

	template<typename T>
	using type_mallocator = type_allocator<T, mallocator>;

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