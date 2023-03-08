#pragma once

#include "shared_fwd.h"
#include "threaded_fwd.h"
#include "type_allocator_fwd.h"

#include <cstddef>

namespace ktl
{
	// linear_allocator
	template<size_t Size>
	class linear_allocator;

	/**
	 * @brief Shorthand for a typed linear allocator
	*/
	template<typename T, size_t Size>
	using type_linear_allocator = type_allocator<T, linear_allocator<Size>>;

	/**
	 * @brief Shorthand for a typed, ref-counted linear allocator
	*/
	template<typename T, size_t Size>
	using type_shared_linear_allocator = type_allocator<T, shared<linear_allocator<Size>>>;

	/**
	 * @brief Shorthand for a typed, thread-safe, ref-counted linear allocator
	*/
	template<typename T, size_t Size>
	using type_threaded_linear_allocator = type_allocator<T, shared_threaded<linear_allocator<Size>>>;
}