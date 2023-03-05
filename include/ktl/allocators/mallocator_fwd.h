#pragma once

#include <cstddef>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
    class type_allocator;
    
	// mallocator
	class mallocator;

	/**
	 * @brief An allocator which uses an aligned malloc for allocation.
	 * @note Like std::allocator it holds no state, so any instance can de/allocate mememory from any other instance
	 * @tparam T The type to use with the allocator
	*/
	template<typename T>
	using type_mallocator = type_allocator<T, mallocator>;
}