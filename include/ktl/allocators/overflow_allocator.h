#pragma once

#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <cstring>
#include <memory>
#include <type_traits>

#include <ostream>

namespace ktl
{
	template<typename Alloc, std::ostream& Stream>
	class overflow_allocator
	{
	private:
		static_assert(has_value_type<Alloc>(), "Building on top of typed allocators is not allowed. Use allocators without a type");

	public:
		typedef typename get_size_type<Alloc>::type size_type;

	private:
		static constexpr int OVERFLOW_PATTERN = 0b01010101010101010101010101010101;
		static constexpr size_t OVERFLOW_SIZE = 64;

		struct stats
		{
			size_type Allocs = 0;
			size_type Constructs = 0;
		};

	public:
		overflow_allocator(const Alloc& alloc = Alloc()) noexcept :
			m_Alloc(alloc),
			m_Stats(std::make_shared<stats>()) {}

		overflow_allocator(const overflow_allocator& other) noexcept :
			m_Alloc(other.m_Alloc),
			m_Stats(other.m_Stats) {}

		~overflow_allocator()
		{
			// Only assume a leak when we're the last reference
			// Otherwise copies could still be using it, but receive an error
			if (m_Stats.use_count() == 1)
			{
				if (m_Stats->Allocs != 0 || m_Stats->Constructs != 0)
					Stream << "--------MEMORY LEAK DETECTED--------\nAllocator destroyed while having:\n" << m_Stats->Allocs << " Allocations\n" << m_Stats->Constructs << " Constructions\n";
			}
		}

#pragma region Allocation
		void* allocate(size_type n)
		{
			m_Stats->Allocs += n;

			size_type size = n + OVERFLOW_SIZE * 2;
			char* ptr = reinterpret_cast<char*>(m_Alloc.allocate(size));

			if (!ptr)
				return nullptr;

			std::memset(ptr, OVERFLOW_PATTERN, OVERFLOW_SIZE);
			std::memset(ptr + OVERFLOW_SIZE + n, OVERFLOW_PATTERN, OVERFLOW_SIZE);

			return ptr + OVERFLOW_SIZE;
		}

		void deallocate(void* p, size_type n)
		{
			m_Stats->Allocs -= n;

			if (p)
			{
				char* ptr = reinterpret_cast<char*>(p);

				// HACK: In reality this should be compared to the reference directly, but that would require more allocation etc...
				// Instead we just compare them to eachother. If corruption has occurred, it's very unlikely to have corrupted in an identical pattern
				if (std::memcmp(ptr - OVERFLOW_SIZE, ptr + n, OVERFLOW_SIZE) != 0)
					Stream << "--------MEMORY CORRUPTION DETECTED--------\nThe area around " << reinterpret_cast<int*>(ptr + OVERFLOW_SIZE) << " has been modified\n";

				size_type size = n + OVERFLOW_SIZE * 2;
				m_Alloc.deallocate(ptr - OVERFLOW_SIZE, size);
			}
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		void construct(T* p, Args&&... args)
		{
			m_Stats->Constructs++;

			if constexpr (has_construct<void, Alloc, T*, Args...>::value)
				m_Alloc.construct(p, std::forward<Args>(args)...);
			else
				::new (p) T(std::forward<Args>(args)...);
		}

		template<typename T>
		void destroy(T* p)
		{
			m_Stats->Constructs--;

			if constexpr (has_destroy<Alloc, T*>::value)
				m_Alloc.destroy(p);
			else
				p->~T();
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<has_max_size<A>::value, size_type>::type
		max_size() const noexcept
		{
			return m_Alloc.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<has_owns<A>::value, bool>::type
		owns(void* p)
		{
			return m_Alloc.owns(p);
		}
#pragma endregion

		Alloc& get_allocator()
		{
			return m_Alloc;
		}

		const Alloc& get_allocator() const
		{
			return m_Alloc;
		}

	private:
		Alloc m_Alloc;
		std::shared_ptr<stats> m_Stats;
	};

	template<typename A, std::ostream& S, typename U, std::ostream& V>
	bool operator==(const overflow_allocator<A, S>& lhs, const overflow_allocator<U, V>& rhs) noexcept
	{
		return &lhs == &rhs;
	}

	template<typename A, std::ostream& S, typename U, std::ostream& V>
	bool operator!=(const overflow_allocator<A, S>& lhs, const overflow_allocator<U, V>& rhs) noexcept
	{
		return &lhs != &rhs;
	}

	template<typename T, typename A, std::ostream& Stream>
	using type_overflow_allocator = type_allocator<T, overflow_allocator<A, Stream>>;
}