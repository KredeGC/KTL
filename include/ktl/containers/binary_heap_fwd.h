#pragma once

#include <memory>
#include <functional>

namespace ktl
{
	template<typename T, typename Comp, typename Alloc = std::allocator<T>>
	class binary_heap;

	/**
	 * @brief An implementation of a binary min heap, using std::less<T>
	 * @tparam T The type to use. Must be move constructible and move assignable
	 * @tparam Alloc The type of allocoator to use
	*/
	template<typename T, typename Alloc = std::allocator<T>>
	using binary_min_heap = binary_heap<T, std::less<T>, Alloc>;

	/**
	 * @brief An implementation of a binary max heap, using std::greater<T>
	 * @tparam T The type to use. Must be move constructible and move assignable
	 * @tparam Alloc The type of allocoator to use
	*/
	template<typename T, typename Alloc = std::allocator<T>>
	using binary_max_heap = binary_heap<T, std::greater<T>, Alloc>;
}