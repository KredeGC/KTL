#pragma once

#include "type_allocator.h"

#include "../utility/alignment_utility.h"

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
			char* Free;
			size_t ObjectCount;

			stack() noexcept :
				Data{},
				Free(Data),
				ObjectCount(0) {}
		};

	public:
		linear_allocator() noexcept :
			m_Block(std::make_shared<arena>()) {}

		linear_allocator(const linear_allocator& other) noexcept :
			m_Block(other.m_Block) {}

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

		size_t max_size() const noexcept
		{
			return Size;
		}

		bool owns(void* p)
		{
			char* ptr = reinterpret_cast<char*>(p);
			return ptr >= m_Block->Data && ptr < m_Block->Data + Size;
		}

	private:
		std::shared_ptr<arena> m_Block;
	};

	template<size_t S, size_t T>
	bool operator==(const linear_allocator<S>& lhs, const linear_allocator<T>& rhs) noexcept
	{
		return &lhs == &rhs;
	}

	template<size_t S, size_t T>
	bool operator!=(const linear_allocator<S>& lhs, const linear_allocator<T>& rhs) noexcept
	{
		return &lhs != &rhs;
	}

	template<typename T, size_t Size>
	using type_linear_allocator = type_allocator<T, linear_allocator<Size>>;
}