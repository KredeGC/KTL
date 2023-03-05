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

	/**
	 * @brief A linear allocator which gives out chunks of its allocated stack.
	 * Increments a counter during allocation, which makes it very fast but also unlikely to deallocate it again.
	 * Has a max allocation size of the @p Size given.
	 * @note Cannot be default constructed because it needs a reference to a stack
	 * @tparam T The type to use with the allocator
	*/
	template<typename T, size_t Size>
	using type_stack_allocator = type_allocator<T, stack_allocator<Size>>;
}