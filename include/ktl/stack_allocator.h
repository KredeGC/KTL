#pragma once

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename T, size_t Size = 4096>
	class stack_allocator
	{
	public:
		using value_type = T;
		using is_always_equal = std::true_type;

		template<typename U>
		struct rebind
		{
			using other = stack_allocator<U>;
		};

		stack_allocator() noexcept : m_Begin(m_Chunk), m_Free(m_Chunk) {}

		template<typename U>
		stack_allocator(const stack_allocator<U>&) noexcept {}

		T* allocate(size_t n)
		{
			if ((size_t(m_Free - m_Begin) + n) > (Size / sizeof(T)))
				return nullptr;

			T* current = m_Free;

			m_Free += n;
			m_ObjectCount += n;

			return current;
		}

		void deallocate(T* p, size_t n)
		{
			if (m_Free - n == p)
				m_Free -= n;

			m_ObjectCount -= n;

			// Assumes that people don't deallocate the same memory twice
			if (m_ObjectCount == 0)
				m_Free = m_Begin;
		}

		bool owns(T* p)
		{
			return p >= m_Begin && p < m_Begin + (Size / sizeof(T));
		}

	private:
		value_type m_Chunk[Size / sizeof(T)];

		value_type* m_Begin;
		value_type* m_Free;

		size_t m_ObjectCount = 0;
	};

	template<typename T, typename U>
	bool operator==(const stack_allocator<T>&, const stack_allocator<U>&) noexcept
	{
		return true;
	}

	template<typename T, typename U>
	bool operator!=(const stack_allocator<T>&, const stack_allocator<U>&) noexcept
	{
		return false;
	}
}