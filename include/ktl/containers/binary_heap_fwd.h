#pragma once

#include <memory>
#include <functional>

namespace ktl
{
	template<typename T, typename Comp, typename Alloc = std::allocator<T>>
	class binary_heap;

	template<typename T, typename Alloc = std::allocator<T>>
	using binary_min_heap = binary_heap<T, std::less<T>, Alloc>;

	template<typename T, typename Alloc = std::allocator<T>>
	using binary_max_heap = binary_heap<T, std::greater<T>, Alloc>;
}