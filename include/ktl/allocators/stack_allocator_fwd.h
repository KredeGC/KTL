#pragma once

#include <cstddef>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
	class type_allocator;
    
	// stack
	template<size_t Size>
	struct stack;
    
	// stack_allocator
	template<size_t Size>
	class stack_allocator;

	template<typename T, size_t Size>
	using type_stack_allocator = type_allocator<T, stack_allocator<Size>>;
}