#pragma once

#include "shared_fwd.h"
#include "threaded_fwd.h"
#include "type_allocator_fwd.h"

#include <cstddef>

namespace ktl
{
	// stack
	template<size_t Size>
	struct stack;
    
	// stack_allocator
	template<size_t Size>
	class stack_allocator;

	/**
	 * @brief Shorthand for a typed stack allocator
	*/
	template<typename T, size_t Size>
	using type_stack_allocator = type_allocator<T, stack_allocator<Size>>;

	/**
	 * @brief Shorthand for a typed, ref-counted stack allocator
	*/
	template<typename T, size_t Size>
	using type_shared_stack_allocator = type_allocator<T, shared<stack_allocator<Size>>>;

	/**
	 * @brief Shorthand for a typed, thread-safe, ref-counted stack allocator
	*/
	template<typename T, size_t Size>
	using type_threaded_stack_allocator = type_allocator<T, threaded<stack_allocator<Size>>>;
}