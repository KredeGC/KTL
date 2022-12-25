#pragma once

#include "../utility/assert_utility.h"
#include "../utility/meta_template.h"
#include "../utility/notomic.h"
#include "overflow_fwd.h"
#include "type_allocator.h"

#include <cstring>
#include <memory>
#include <ostream>
#include <type_traits>

namespace ktl
{
	template<typename Alloc, std::ostream& Stream, typename Atomic>
	class overflow
	{
	private:
		static_assert(has_no_value_type<Alloc>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");

	public:
		typedef typename get_size_type<Alloc>::type size_type;

	private:
		static constexpr int OVERFLOW_PATTERN = 0b01010101010101010101010101010101;
		static constexpr int64_t OVERFLOW_TEST = 0b0101010101010101010101010101010101010101010101010101010101010101;
		static constexpr size_t OVERFLOW_SIZE = 8;

		struct stats
		{
			Alloc Allocator;
			Atomic UseCount;
			size_type Allocs;
			size_type Constructs;

			stats(const Alloc& alloc) :
				Allocator(alloc),
				UseCount(1),
				Allocs(0),
				Constructs(0) {}
		};

	public:
		overflow(const Alloc& alloc = Alloc()) noexcept
		{
			// Allocate the control block with the allocator, if we fit
			if constexpr (!has_max_size<Alloc>::value)
			{
				m_Stats = reinterpret_cast<stats*>(const_cast<Alloc&>(alloc).allocate(sizeof(stats)));
				if constexpr (has_construct<void, Alloc, stats*, const Alloc&>::value)
					alloc.construct(m_Stats, alloc);
				else
					::new(m_Stats) stats(alloc);
			}
			else
			{
				m_Stats = new stats(alloc);
			}
		}

		overflow(const overflow& other) noexcept :
			m_Stats(other.m_Stats)
		{
			m_Stats->UseCount++;
		}

		overflow(overflow&& other) noexcept :
			m_Stats(other.m_Stats)
		{
			KTL_ASSERT(other.m_Stats);
			other.m_Stats = nullptr;
		}

		~overflow()
		{
			if (m_Stats)
				decrement();
		}

		overflow& operator=(const overflow& rhs) noexcept
		{
			if (m_Stats)
				decrement();

			m_Stats = rhs.m_Stats;
			m_Stats->UseCount++;

			return *this;
		}

		overflow& operator=(overflow&& rhs) noexcept
		{
			if (m_Stats)
				decrement();

			m_Stats = rhs.m_Stats;

			rhs.m_Stats = nullptr;

			return *this;
		}

		bool operator==(const overflow& rhs) const noexcept
		{
			return m_Stats->Allocator == rhs.m_Stats->Allocator;
		}

		bool operator!=(const overflow& rhs) const noexcept
		{
			return m_Stats->Allocator != rhs.m_Stats->Allocator;
		}

#pragma region Allocation
		void* allocate(size_type n)
		{
			m_Stats->Allocs += n;

			size_type size = n + OVERFLOW_SIZE * 2;
			char* ptr = reinterpret_cast<char*>(m_Stats->Allocator.allocate(size));

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

				// Check against corruption
				if (std::memcmp(ptr + n, &OVERFLOW_TEST, OVERFLOW_SIZE) != 0)
					Stream << "--------MEMORY CORRUPTION DETECTED--------\nThe area around " << reinterpret_cast<int*>(ptr + n) << " has been modified\n";

				if (std::memcmp(ptr - OVERFLOW_SIZE, &OVERFLOW_TEST, OVERFLOW_SIZE) != 0)
					Stream << "--------MEMORY CORRUPTION DETECTED--------\nThe area around " << reinterpret_cast<int*>(ptr - OVERFLOW_SIZE) << " has been modified\n";

				size_type size = n + OVERFLOW_SIZE * 2;
				m_Stats->Allocator.deallocate(ptr - OVERFLOW_SIZE, size);
			}
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		void construct(T* p, Args&&... args)
		{
			m_Stats->Constructs++;

			if constexpr (has_construct<void, Alloc, T*, Args...>::value)
				m_Stats->Allocator.construct(p, std::forward<Args>(args)...);
			else
				::new(p) T(std::forward<Args>(args)...);
		}

		template<typename T>
		void destroy(T* p)
		{
			m_Stats->Constructs--;

			if constexpr (has_destroy<Alloc, T*>::value)
				m_Stats->Allocator.destroy(p);
			else
				p->~T();
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<has_max_size<A>::value, size_type>::type
		max_size() const noexcept
		{
			return m_Stats->Allocator.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<has_owns<A>::value, bool>::type
		owns(void* p)
		{
			return m_Stats->Allocator.owns(p);
		}
#pragma endregion

		Alloc& get_allocator()
		{
			return m_Stats->Allocator;
		}

		const Alloc& get_allocator() const
		{
			return m_Stats->Allocator;
		}

	private:
		void decrement()
		{
			if (m_Stats->UseCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				if (m_Stats->Allocs != 0 || m_Stats->Constructs != 0)
					Stream << "--------MEMORY LEAK DETECTED--------\nAllocator destroyed while having:\n" << m_Stats->Allocs << " Allocations\n" << m_Stats->Constructs << " Constructions\n";

				if constexpr (!has_max_size<Alloc>::value)
				{
					Alloc alloc = std::move(m_Stats->Allocator);

					if constexpr (has_destroy<Alloc, stats*>::value)
						alloc.destroy(m_Stats);
					else
						m_Stats->~stats();
					alloc.deallocate(m_Stats, sizeof(stats));
				}
				else
				{
					delete m_Stats;
				}
			}
		}

		stats* m_Stats;
	};
}