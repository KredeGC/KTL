#pragma once

#include "../utility/alignment_utility.h"
#include "stack_allocator_fwd.h"
#include "type_allocator.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Size>
	struct stack
	{
		alignas(ALIGNMENT) char Data[Size];
		char* Free;
		size_t ObjectCount;

		stack() noexcept :
			Data{},
			Free(Data),
			ObjectCount(0) {}
	};

	template<size_t Size>
	class stack_allocator
	{
	public:
		stack_allocator(stack<Size>& block) noexcept :
			m_Block(&block) {}

		stack_allocator(stack<Size>* block) noexcept
			: m_Block(block) {}

		stack_allocator(const stack_allocator& other) noexcept = default;

		stack_allocator(stack_allocator&& other) noexcept = default;

		stack_allocator& operator=(const stack_allocator& other) noexcept = default;

		stack_allocator& operator=(stack_allocator&& other) noexcept = default;

		bool operator==(const stack_allocator& rhs) const noexcept
		{
			return m_Block == rhs.m_Block;
		}

		bool operator!=(const stack_allocator& rhs) const noexcept
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
		stack<Size>* m_Block;
	};
}