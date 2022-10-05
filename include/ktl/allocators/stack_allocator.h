#pragma once

#include "type_allocator.h"

#include "../utility/alignment_utility.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Size>
	struct stack
	{
		char* Free;
		alignas(ALIGNMENT) char Data[Size];
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
		using size_type = size_t;

		stack_allocator(stack<Size>& block) noexcept :
			m_Block(&block) {}

		stack_allocator(const stack_allocator& other) noexcept :
			m_Block(other.m_Block) {}

		[[nodiscard]] void* allocate(size_type n)
		{
			size_t totalSize = n + align_to_architecture(n);

			if ((size_t(m_Block->Free - m_Block->Data) + totalSize) > Size)
				return nullptr;

			char* current = m_Block->Free;

			m_Block->Free += totalSize;
			m_Block->ObjectCount += totalSize;

			return current;
		}

		void deallocate(void* p, size_type n) noexcept
		{
			size_t totalSize = n + align_to_architecture(n);

			if (m_Block->Free - totalSize == p)
				m_Block->Free -= totalSize;

			m_Block->ObjectCount -= totalSize;

			// Assumes that people don't deallocate the same memory twice
			if (m_Block->ObjectCount == 0)
				m_Block->Free = m_Block->Data;
		}

		size_type max_size() const noexcept
		{
			return Size;
		}

		bool owns(void* p)
		{
			char* ptr = reinterpret_cast<char*>(p);
			return ptr >= m_Block->Data && ptr < m_Block->Data + Size;
		}

		// TEMP
		template<typename T, typename... Args>
		void construct(T* p, Args&&... args)
		{
			::new (static_cast<void*>(p)) T(std::forward<Args>(args)...);
		}

		template<typename T>
		void destroy(T* p)
		{
			p->~T();
		}
		// TEMP

	private:
		stack<Size>* m_Block;
	};

	template<size_t S, size_t T>
	bool operator==(const stack_allocator<S>& lhs, const stack_allocator<T>& rhs) noexcept
	{
		return &lhs == &rhs;
	}

	template<size_t S, size_t T>
	bool operator!=(const stack_allocator<S>& lhs, const stack_allocator<T>& rhs) noexcept
	{
		return &lhs != &rhs;
	}

	template<typename T, size_t Size>
	using type_stack_allocator = type_allocator<T, stack_allocator<Size>>;
}