#pragma once

#include <cstring>
#include <memory>
#include <type_traits>

#include <iostream>

namespace ktl
{
	template<typename Alloc, std::ostream& Stream = std::cerr>
	class overflow_allocator
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

		overflow_allocator(const Alloc& alloc = Alloc()) noexcept: m_Alloc(alloc) {}

		overflow_allocator(const overflow_allocator& other) noexcept :
			m_Alloc(other.m_Alloc),
			m_Allocs(other.m_Allocs),
			m_Constructs(other.m_Constructs) {}

		template<typename A, std::ostream& S>
		overflow_allocator(const overflow_allocator<A, S>& other) noexcept :
			m_Alloc(other.m_Alloc),
			m_Allocs(other.m_Allocs),
			m_Constructs(other.m_Constructs) {}

		~overflow_allocator()
		{
			if (m_Allocs != 0 || m_Constructs != 0)
				Stream << "--------MEMORY LEAK DETECTED--------\nAllocator destroyed while having:\n" << m_Allocs << " Allocations\n" << m_Constructs << " Constructions\n";
		}

		value_type* allocate(size_t n)
		{
			m_Allocs += n;

			 size_t overflowSize = n * 3;
			 value_type* ptr = traits::allocate(m_Alloc, overflowSize);

			 std::memset(ptr, 31, n);
			 std::memset(ptr + 2 * n, 31, n);

			return ptr + n;
		}

		void deallocate(value_type* p, size_t n)
		{
			m_Allocs -= n;

			 // HACK: In reality this should be compared to the reference directly, but that would require more allocation etc...
			 // Instead we just compare them to eachother. If corruption has occurred, it's very unlikely to have corrupted similarly in both blocks
			 if (std::memcmp(p - n, p + n, n) != 0)
			 	Stream << "--------MEMORY CORRUPTION DETECTED--------\nThe area around " << reinterpret_cast<int*>(p + n) << " has been modified\n";

			 size_t overflowSize = n * 3;

			traits::deallocate(m_Alloc, p - n, overflowSize);
		}

		template<class... Args>
		void construct(value_type* p, Args&&... args)
		{
			m_Constructs++;

			traits::construct(m_Alloc, p, std::forward<Args>(args)...);
		}

		void destroy(value_type* p)
		{
			m_Constructs--;

			traits::destroy(m_Alloc, p);
		}

	private:
		Alloc m_Alloc;

		size_t m_Allocs = 0;
		size_t m_Constructs = 0;
	};

	template<typename A, std::ostream& S, typename U, std::ostream& V>
	bool operator==(const overflow_allocator<A, S>&, const overflow_allocator<U, V>&) noexcept
	{
		return true;
	}

	template<typename A, std::ostream& S, typename U, std::ostream& V>
	bool operator!=(const overflow_allocator<A, S>&, const overflow_allocator<U, V>&) noexcept
	{
		return false;
	}
}