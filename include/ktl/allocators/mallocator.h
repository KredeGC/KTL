#pragma once

#include "../utility/aligned_malloc.h"
#include "../utility/alignment.h"
#include "mallocator_fwd.h"
#include "type_allocator.h"

namespace ktl
{
	/**
	 * @brief An allocator which uses an aligned malloc for allocation.
	 * @note Like std::allocator it holds no state, so any instance can de/allocate mememory from any other instance.
	 * Similar to std::allocator, except it's untyped.
	*/
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
		void* allocate(size_t n) noexcept
		{
			return detail::aligned_malloc(n, detail::ALIGNMENT);
		}

		void deallocate(void* p, size_t n) noexcept
		{
			detail::aligned_free(p);
		}
#pragma endregion
	};
}