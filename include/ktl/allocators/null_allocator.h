#pragma once

#include "../utility/assert.h"

namespace ktl
{
	/**
	 * @brief An allocator which does nothing.
	 * Useful for debugging composite allocators, like ensuring a specific path in a composite allocator is not used.
	*/
	class null_allocator
	{
	public:
		null_allocator() noexcept = default;

		null_allocator(const null_allocator&) noexcept = default;

		null_allocator(null_allocator&&) noexcept = default;

		null_allocator& operator=(const null_allocator&) noexcept = default;

		null_allocator& operator=(null_allocator&&) noexcept = default;

		bool operator==(const null_allocator& rhs) const noexcept
		{
			return true;
		}

		bool operator!=(const null_allocator& rhs) const noexcept
		{
			return false;
		}

#pragma region Allocation
		void* allocate(size_t n)
		{
			return nullptr;
		}

		void deallocate(void* p, size_t n) noexcept
		{
			KTL_ASSERT(p == nullptr);
		}
#pragma endregion

#pragma region Utility
		bool owns(void* p) const
		{
			return p == nullptr;
		}
#pragma endregion
	};
}