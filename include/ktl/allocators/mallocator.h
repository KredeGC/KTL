#pragma once

#include "type_allocator.h"

#include "../utility/alignment_malloc.h"
#include "../utility/alignment_utility.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	class mallocator
	{
	public:
		mallocator() noexcept = default;

		mallocator(const mallocator& other) noexcept = default;

		void* allocate(size_t n)
		{
			return aligned_malloc(n, ALIGNMENT);
		}

		void deallocate(void* p, size_t n) noexcept
		{
			aligned_free(p);
		}

		bool operator==(const mallocator& rhs) noexcept
		{
			return true;
		}

		bool operator!=(const mallocator& rhs) noexcept
		{
			return false;
		}
	};

	template<typename T>
	using type_mallocator = type_allocator<T, mallocator>;
}