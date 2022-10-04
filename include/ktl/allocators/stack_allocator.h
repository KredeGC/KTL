#pragma once

#include "type_allocator.h"

#include "../utility/alignment_utility.h"
#include "../utility/stack_type.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	class stack_allocator
	{
	public:
		using size_type = size_t;

		template<size_t Size>
		stack_allocator(stack<Size>& block) noexcept :
			m_Size(Size)
		{
			m_Block = block.Data;

			m_Block += align_to_architecture(size_t(m_Block));

			m_Free = reinterpret_cast<char*>(m_Block);
		}

		stack_allocator(const stack_allocator& other) noexcept :
			m_Block(other.m_Block),
			m_Free(other.m_Free),
			m_Size(other.m_Size),
			m_ObjectCount(other.m_ObjectCount) {}

		[[nodiscard]] void* allocate(size_type n)
		{
			if ((size_t(m_Free - m_Block) + n) > m_Size)
				return nullptr;

			char* current = m_Free;

			m_Free += n;
			m_ObjectCount += n;

			return current;
		}

		void deallocate(void* p, size_type n) noexcept
		{
			if (m_Free - n == p)
				m_Free -= n;

			m_ObjectCount -= n;

			// Assumes that people don't deallocate the same memory twice
			if (m_ObjectCount == 0)
				m_Free = m_Block;
		}

		size_type max_size() const noexcept
		{
			return m_Size;
		}

		bool owns(void* p)
		{
			char* ptr = reinterpret_cast<char*>(p);
			return ptr >= m_Block && ptr < m_Block + m_Size;
		}

		bool operator==(const stack_allocator& rhs) const noexcept
		{
			return m_Block == rhs.m_Block;
		}

		bool operator!=(const stack_allocator& rhs) const noexcept
		{
			return m_Block != rhs.m_Block;
		}

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

	private:
		char* m_Block;
		char* m_Free;

		size_t m_Size;
		size_t m_ObjectCount = 0;
	};

	template<typename T>
	using stack_type_allocator = type_allocator<T, stack_allocator>;
}