#pragma once

#include <atomic>
#include <cstddef>

namespace ktl
{
    // Wrapper class for making allocator typed
	template<typename T, typename Alloc>
	class type_allocator;

	// Wrapper class for making allocator ref-counted
	template<typename Alloc, typename Atomic>
	class shared;

	// Wrapper class for making allocator thread-safe
	template<typename Alloc>
	class threaded;

	// Fake atomic counter
	template<typename T>
	struct notomic;

	// linear_allocator
	template<size_t Size>
	class linear_allocator;

	/**
	 * @brief Shorthand for a ref-counted linear allocator
	*/
	template<size_t Size>
	using shared_linear_allocator = shared<linear_allocator<Size>, notomic<size_t>>;

	/**
	 * @brief Shorthand for a thead-safe, ref-counted linear allocator
	*/
	template<size_t Size>
	using threaded_linear_allocator = shared<threaded<linear_allocator<Size>>, std::atomic<size_t>>;

	/**
	 * @brief Shorthand for a typed linear allocator
	*/
	template<typename T, size_t Size>
	using type_linear_allocator = type_allocator<T, linear_allocator<Size>>;

	/**
	 * @brief Shorthand for a typed, ref-counted linear allocator
	*/
	template<typename T, size_t Size>
	using type_shared_linear_allocator = type_allocator<T, shared_linear_allocator<Size>>;

	/**
	 * @brief Shorthand for a typed, thread-safe, ref-counted linear allocator
	*/
	template<typename T, size_t Size>
	using type_threaded_linear_allocator = type_allocator<T, threaded_linear_allocator<Size>>;
}