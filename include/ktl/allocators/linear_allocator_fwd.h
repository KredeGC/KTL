#pragma once

#include <cstddef>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
	class type_allocator;

	// stl_allocator
	template<typename Alloc>
	class stl_allocator;
    
    // linear_allocator
    template<size_t Size>
	class linear_allocator;

	/**
     * @brief A linear allocator which gives out chunks of its internal stack.
	 * Increments a counter during allocation, which makes it very fast but also unlikely to deallocate it again.
	 * Has a max allocation size of the @p Size given.
	 * @note Shorthand for writing type_allocator<T, linear_allocator<Size>>
	 * @tparam T The type to use with the allocator
	*/
	template<typename T, size_t Size>
	using type_linear_allocator = type_allocator<T, linear_allocator<Size>>;

	/**
	 * @brief A linear allocator which gives out chunks of its internal stack.
	 * Increments a counter during allocation, which makes it very fast but also unlikely to deallocate it again.
	 * Has a max allocation size of the @p Size given.
	 * @note Shorthand for writing type_allocator<T, stl_allocator<linear_allocator<Size>>>
	 * @tparam T The type to use with the allocator
	*/
	template<typename T, size_t Size>
	using type_stl_linear_allocator = type_allocator<T, stl_allocator<linear_allocator<Size>>>;
}