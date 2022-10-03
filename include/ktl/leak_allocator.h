#pragma once

#include <memory>
#include <type_traits>

#include <iostream>

namespace ktl
{
	template<typename Alloc, std::ostream& Stream = std::cerr>
	class leak_allocator : private Alloc
	{
	public:
		using value_type = typename Alloc::value_type;
		using is_always_equal = std::true_type;

		using traits = std::allocator_traits<Alloc>;

		template<typename U, std::ostream& V>
		struct rebind
		{
			using other = leak_allocator<U, V>;
		};

		leak_allocator() noexcept : Alloc() {}

		template<typename U, std::ostream& V>
		leak_allocator(const leak_allocator<U, V>&) noexcept {}

		~leak_allocator()
		{
			if (m_Allocs != 0 || m_Constructs != 0)
				Stream << "--------MEMORY LEAK DETECTED--------\nAllocator destroyed while having:\n" << m_Allocs << " Allocations\n" << m_Constructs << " Constructions\n";
		}

		value_type* allocate(size_t n)
		{
			m_Allocs += n;

			return traits::allocate(*this, n);
		}

		void deallocate(value_type* p, size_t n)
		{
			m_Allocs -= n;

			traits::deallocate(*this, p, n);
		}

		template<class... Args>
		void construct(value_type* p, Args&&... args)
		{
			m_Constructs++;

			traits::construct(*this, p, std::forward<Args>(args)...);
		}

		void destroy(value_type* p)
		{
			m_Constructs--;

			traits::destroy(*this, p);
		}

	private:
		size_t m_Allocs = 0;
		size_t m_Constructs = 0;
	};

	template<typename T, typename U>
	bool operator==(const leak_allocator<T>&, const leak_allocator<U>&) noexcept
	{
		return true;
	}

	template<typename T, typename U>
	bool operator!=(const leak_allocator<T>&, const leak_allocator<U>&) noexcept
	{
		return false;
	}
}