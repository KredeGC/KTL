#pragma once

#include <memory>
#include <type_traits>

#include <iostream>

namespace ktl
{
	template<typename Alloc, std::ostream& Stream = std::cerr>
	class overflow_allocator : private Alloc
	{
	private:
		using traits = std::allocator_traits<Alloc>;

	public:
		using value_type = typename Alloc::value_type;
		using is_always_equal = std::true_type;

		template<typename U, std::ostream& V>
		struct rebind
		{
			using other = overflow_allocator<U, V>;
		};

		overflow_allocator() noexcept : Alloc() {}

		template<typename U, std::ostream& V>
		overflow_allocator(const overflow_allocator<U, V>&) noexcept {}

		~overflow_allocator()
		{
			if (m_Allocs != 0 || m_Constructs != 0)
				Stream << "--------MEMORY LEAK DETECTED--------\nAllocator destroyed while having:\n" << m_Allocs << " Allocations\n" << m_Constructs << " Constructions\n";
		}

		value_type* allocate(size_t n)
		{
			m_Allocs += n;

			size_t overflowSize = n * 3;
			value_type* ptr = traits::allocate(*this, overflowSize);

			memset(ptr, 0, n);
			memset(ptr + 2 * n, 0, n);

			return ptr + n;
		}

		void deallocate(value_type* p, size_t n)
		{
			m_Allocs -= n;

			// HACK: In reality this should be compared to 0 directly, but that would require more allocation etc...
			// Instead we just compare them to eachother. If corruption has occurred, it's very unlikely to have corrupted similarly in both blocks
			if (memcmp(p - n, p + n, n) != 0)
				Stream << "--------MEMORY CORRUPTION DETECTED--------\nThe area around " << p << " has been modified\n";

			size_t overflowSize = n * 3;

			traits::deallocate(*this, p - n, overflowSize);
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
	bool operator==(const overflow_allocator<T>&, const overflow_allocator<U>&) noexcept
	{
		return true;
	}

	template<typename T, typename U>
	bool operator!=(const overflow_allocator<T>&, const overflow_allocator<U>&) noexcept
	{
		return false;
	}
}