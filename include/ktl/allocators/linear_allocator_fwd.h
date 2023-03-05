#pragma once

#include <cstddef>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
	class type_allocator;
    
    // notomic
    template<typename T>
    struct notomic;
    
    // linear_allocator
    template<size_t Size, typename Atomic = notomic<size_t>>
	class linear_allocator;

	/**
     * @brief A linear allocator which gives out chunks of its internal stack.
	 * Increments a counter during allocation, which makes it very fast but also unlikely to deallocate it again.
	 * Has a max allocation size of the @p Size given.
	 * @tparam T The type to use with the allocator
     * @tparam Atomic The atomic type to use for reference counting. Threading is not implemented, so this defaults to ktl::notomic<size_t>
	*/
	template<typename T, size_t Size, typename Atomic = notomic<size_t>>
	using type_linear_allocator = type_allocator<T, linear_allocator<Size, Atomic>>;
}