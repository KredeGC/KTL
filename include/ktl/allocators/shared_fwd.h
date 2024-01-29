#pragma once

#include "type_allocator_fwd.h"

#include <atomic>

namespace ktl
{
	// Wrapper class for an integer with std::atomic syntax
	template<typename T>
	class notomic;

	// Wrapper class for making allocator ref-counted
	template<typename Alloc, template<typename> typename Atomic = notomic>
	class shared;

	/**
	 * @brief Shorthand for an atomic-ref-counted allocator
	*/
	template<typename Alloc>
	using atomic_shared = shared<Alloc, std::atomic>;

	/**
	 * @brief Shorthand for a typed, ref-counted allocator
	*/
	template<typename T, typename Alloc>
	using type_shared = type_allocator<T, shared<Alloc>>;

	/**
	 * @brief Shorthand for a typed, atomic-ref-counted allocator
	*/
	template<typename T, typename Alloc>
	using type_atomic_shared = type_allocator<T, atomic_shared<Alloc>>;
}