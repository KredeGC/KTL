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
    
    // list_allocator
	template<size_t Size, typename Alloc, typename Atomic = notomic<size_t>>
	class list_allocator;

	template<typename T, size_t Size, typename Alloc, typename Atomic = notomic<size_t>>
	using type_list_allocator = type_allocator<T, list_allocator<Size, Alloc, Atomic>>;
    
    // get minimum size of linked list allocator
    template<size_t Size, typename Alloc, typename Atomic = notomic<size_t>>
    struct list_min_size
    {
        inline static constexpr size_t value = Size + sizeof(Alloc) + sizeof(Atomic) + sizeof(void*) * 2;
    };
}