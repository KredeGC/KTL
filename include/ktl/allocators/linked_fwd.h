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
    
    // linked
	template<size_t Size, typename Alloc, typename Atomic = notomic<size_t>>
	class linked;

	template<typename T, size_t Size, typename Alloc, typename Atomic = notomic<size_t>>
	using type_linked_allocator = type_allocator<T, linked<Size, Alloc, Atomic>>;
    
    // get minimum size of linked list allocator
    template<size_t Size, typename Alloc, typename Atomic = notomic<size_t>>
    struct list_min_size
    {
        inline static constexpr size_t value = Size + sizeof(Alloc) + sizeof(Atomic) + sizeof(void*) * 2;
    };
}