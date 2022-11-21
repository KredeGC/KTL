#pragma once

#include "../utility/assert_utility.h"
#include "../utility/alignment_utility.h"
#include "type_allocator.h"

#include <atomic>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Size>
	class linear_allocator
	{
	private:
		struct arena
		{
			alignas(ALIGNMENT) char Data[Size];
			std::atomic<size_t> UseCount;
			char* Free;
			size_t ObjectCount;

			arena() noexcept :
				Data{},
				UseCount(1),
				Free(Data),
				ObjectCount(0) {}
		};

	public:
		linear_allocator() noexcept
		{
			m_Block = new arena;
		}

		linear_allocator(const linear_allocator& other) noexcept :
			m_Block(other.m_Block)
		{
			m_Block->UseCount++;
		}

		linear_allocator(linear_allocator&& other) noexcept :
			m_Block(other.m_Block)
		{
			KTL_ASSERT(other.m_Block);
			other.m_Block = nullptr;
		}

		~linear_allocator()
		{
			if (m_Block)
				decrement();
		}

		linear_allocator& operator=(const linear_allocator& rhs) noexcept
		{
			if (m_Block)
				decrement();

			m_Block = rhs.m_Block;
			m_Block->UseCount++;

			return *this;
		}

		linear_allocator& operator=(linear_allocator&& rhs) noexcept
		{
			if (m_Block)
				decrement();

			m_Block = rhs.m_Block;

			rhs.m_Block = nullptr;

			return *this;
		}

		bool operator==(const linear_allocator& rhs) const noexcept
		{
			return m_Block == rhs.m_Block;
		}

		bool operator!=(const linear_allocator& rhs) const noexcept
		{
			return m_Block != rhs.m_Block;
		}

#pragma region Allocation
		void* allocate(size_t n)
		{
			size_t totalSize = n + align_to_architecture(n);

			if ((size_t(m_Block->Free - m_Block->Data) + totalSize) > Size)
				return nullptr;

			char* current = m_Block->Free;

			m_Block->Free += totalSize;
			m_Block->ObjectCount += totalSize;

			return current;
		}

		void deallocate(void* p, size_t n) noexcept
		{
			size_t totalSize = n + align_to_architecture(n);

			if (m_Block->Free - totalSize == p)
				m_Block->Free -= totalSize;

			m_Block->ObjectCount -= totalSize;

			// Assumes that people don't deallocate the same memory twice
			if (m_Block->ObjectCount == 0)
				m_Block->Free = m_Block->Data;
		}
#pragma endregion

#pragma region Utility
		size_t max_size() const noexcept
		{
			return Size;
		}

		bool owns(void* p)
		{
			char* ptr = reinterpret_cast<char*>(p);
			return ptr >= m_Block->Data && ptr < m_Block->Data + Size;
		}
#pragma endregion

	private:
		void decrement()
		{
			if (m_Block->UseCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
				delete m_Block;
		}

		arena* m_Block;
	};

	template<typename T, size_t Size>
	using type_linear_allocator = type_allocator<T, linear_allocator<Size>>;
}