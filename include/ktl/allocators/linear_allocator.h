#pragma once

#include "../utility/assert.h"
#include "../utility/alignment.h"
#include "linear_allocator_fwd.h"

#include <memory>
#include <type_traits>

namespace ktl
{
    /**
     * @brief A linear allocator which gives out chunks of its internal stack.
	 * Increments a counter during allocation, which makes it very fast but also unlikely to deallocate it again.
	 * Has a max allocation size of the @p Size given.
    */
    template<size_t Size>
	class linear_allocator
	{
	public:
		linear_allocator() noexcept :
			m_Data{},
			m_Free(m_Data),
			m_ObjectCount(0) {}

		linear_allocator(const linear_allocator&) noexcept = delete;

		/**
		 * @brief Move constructor
		 * @note Moving is only allowed if the original allocator has no allocations
		 * @param other The original allocator
		*/
		linear_allocator(linear_allocator&& other) noexcept :
			m_Data{},
			m_Free(m_Data),
			m_ObjectCount(0)
		{
			// Moving raw allocators in use is undefined
			KTL_ASSERT(other.m_ObjectCount == 0);
		}

		linear_allocator& operator=(const linear_allocator&) noexcept = delete;

		/**
		 * @brief Move assignment operator
		 * @note Moving is only allowed if the original allocator has no allocations
		 * @param rhs The original allocator
		*/
		linear_allocator& operator=(linear_allocator&& rhs) noexcept
		{
			m_Free = m_Data;

			// Moving raw allocators in use is undefined
			KTL_ASSERT(rhs.m_ObjectCount == 0);

			return *this;
		}

		bool operator==(const linear_allocator& rhs) const noexcept
		{
			return m_Data == rhs.m_Data;
		}

		bool operator!=(const linear_allocator& rhs) const noexcept
		{
			return m_Data != rhs.m_Data;
		}

#pragma region Allocation
		/**
		 * @brief Attempts to allocate a chunk of memory defined by @p n
		 * @param n The amount of bytes to allocate memory for
		 * @return A location in memory that is at least @p n bytes big or nullptr if it could not be allocated
		*/
		void* allocate(size_t n)
		{
			size_t totalSize = n + detail::align_to_architecture(n);

			if ((size_t(m_Free - m_Data) + totalSize) > Size)
				return nullptr;

			char* current = m_Free;

			m_Free += totalSize;
			m_ObjectCount += totalSize;

			return current;
		}

		/**
		 * @brief Attempts to deallocate the memory at location @p p
		 * @note The memory is only completely deallocated if it was the last allocation made or all memory has been deallocated
		 * @param p The location in memory to deallocate
		 * @param n The size that was initially allocated
		*/
		void deallocate(void* p, size_t n) noexcept
		{
			KTL_ASSERT(p != nullptr);

			size_t totalSize = n + detail::align_to_architecture(n);

			if (m_Free - totalSize == p)
				m_Free -= totalSize;

			m_ObjectCount -= totalSize;

			// Assumes that people don't deallocate the same memory twice
			if (m_ObjectCount == 0)
				m_Free = m_Data;
		}
#pragma endregion

#pragma region Utility
		/**
		 * @brief Returns the maximum size that an allocation can be
		 * @return The maximum size an allocation may be
		*/
		size_t max_size() const noexcept
		{
			return Size;
		}

		/**
		 * @brief Returns whether or not the allocator owns the given location in memory
		 * @param p The location of the object in memory
		 * @return Whether the allocator owns @p p
		*/
		bool owns(void* p) const
		{
			return p >= m_Data && p < m_Data + Size;
		}
#pragma endregion

	private:
		alignas(detail::ALIGNMENT) char m_Data[Size];
		char* m_Free;
		size_t m_ObjectCount;
	};
}