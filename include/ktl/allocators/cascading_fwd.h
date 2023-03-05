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
    
    // cascading
	template<typename Alloc, typename Atomic = notomic<size_t>>
	class cascading;

	/**
	 * @brief An allocator which owns multiple instances of a given sub allocator.
	 * When allocating it will attempt to use the first available allocator.
	 * Upon failure it will instead create a new instance and allocate from it.
	 * @note Deallocation can take O(n) time as it may have to traverse multiple allocator instances to find the right one.
	 * The allocator type must be default-constructible, which means any variation of stack_allocator can't be used.
	 * The allocator type must also have an owns(*ptr) method, which means any variation of mallocator can't be used.
	 * @tparam T The type to use with the allocator
	 * @tparam Alloc The allocator type to create instances from
	 * @tparam Atomic The atomic type to use for reference counting. Threading is not implemented, so this defaults to ktl::notomic<size_t>
	*/
	template<typename T, typename Alloc, typename Atomic = notomic<size_t>>
	using type_cascading_allocator = type_allocator<T, cascading<Alloc, Atomic>>;
}