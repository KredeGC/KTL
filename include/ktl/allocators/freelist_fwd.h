#pragma once

#include "shared_fwd.h"
#include "threaded_fwd.h"
#include "type_allocator_fwd.h"

#include <cstddef>

namespace ktl
{
	// freelist
	template<size_t Min, size_t Max, typename Alloc>
	class freelist;

	/**
	 * @brief Shorthand for a typed freelist allocator
	*/
	template<typename T, size_t Min, size_t Max, typename Alloc>
	using type_freelist_allocator = type_allocator<T, freelist<Min, Max, Alloc>>;

	/**
	 * @brief Shorthand for a typed, ref-counted freelist allocator
	*/
	template<typename T, size_t Min, size_t Max, typename Alloc>
	using type_shared_freelist_allocator = type_allocator<T, shared<freelist<Min, Max, Alloc>>>;

	/**
	 * @brief Shorthand for a typed, thread-safe, ref-counted freelist allocator
	*/
	template<typename T, size_t Min, size_t Max, typename Alloc>
	using type_threaded_freelist_allocator = type_allocator<T, threaded<freelist<Min, Max, Alloc>>>;
}