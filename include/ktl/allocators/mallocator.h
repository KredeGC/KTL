#pragma once

#include "../utility/aligned_malloc.h"
#include "../utility/alignment.h"
#include "mallocator_fwd.h"
#include "type_allocator.h"

namespace ktl
{
	class mallocator
	{
	public:
		mallocator() noexcept = default;

		mallocator(const mallocator&) noexcept = default;

		mallocator(mallocator&&) noexcept = default;

		mallocator& operator=(const mallocator&) noexcept = default;

		mallocator& operator=(mallocator&&) noexcept = default;

		bool operator==(const mallocator& rhs) const noexcept
		{
			return true;
		}

		bool operator!=(const mallocator& rhs) const noexcept
		{
			return false;
		}

#pragma region Allocation
		void* allocate(size_t n)
		{
			return aligned_malloc(n, ALIGNMENT);
		}

		void deallocate(void* p, size_t n) noexcept
		{
			aligned_free(p);
		}
#pragma endregion
	};
}