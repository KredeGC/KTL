#pragma once

#include <memory>
#include <type_traits>
#include <unordered_set>

namespace ktl
{
	template<typename T>
	class mallocator
	{
	public:
		using value_type = T;
		using is_always_equal = std::true_type;

		template<typename U>
		struct rebind
		{
			using other = mallocator<U>;
		};

		mallocator() noexcept = default;

		mallocator(const mallocator& other) noexcept : m_Allocs(other.m_Allocs) {}

		template<typename U>
		mallocator(const mallocator<U>&) noexcept {}

		T* allocate(size_t n)
		{
			T* ptr = static_cast<T*>(::operator new(sizeof(T) * n));

			m_Allocs.insert(ptr);

			return ptr;
		}

		void deallocate(T* p, size_t n)
		{
			m_Allocs.erase(m_Allocs.find(p));
		}

		bool owns(T* p)
		{
			return m_Allocs.find(p) != m_Allocs.end();
		}

	private:
		std::unordered_set<T*> m_Allocs;
	};

	template<typename T, typename U>
	bool operator==(const mallocator<T>&, const mallocator<U>&) noexcept
	{
		return true;
	}

	template<typename T, typename U>
	bool operator!=(const mallocator<T>&, const mallocator<U>&) noexcept
	{
		return false;
	}
}