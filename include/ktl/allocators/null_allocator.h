#pragma once

#include "../utility/assert_utility.h"

namespace ktl
{
	class null_allocator
	{
	public:
		null_allocator() noexcept = default;

		null_allocator(const null_allocator& other) noexcept = default;

		null_allocator(null_allocator&& other) noexcept = default;

		null_allocator& operator=(const null_allocator& rhs) noexcept = default;

		null_allocator& operator=(null_allocator&& rhs) noexcept = default;

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
		bool owns(void* p)
		{
			return p == nullptr;
		}
#pragma endregion
	};
}