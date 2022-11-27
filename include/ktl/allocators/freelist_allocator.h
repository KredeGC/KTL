#pragma once

#include "../utility/assert_utility.h"
#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Min, size_t Max, typename Alloc>
	class freelist_allocator
	{
	private:
		static_assert(has_no_value_type<Alloc>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");

	public:
		typedef typename get_size_type<Alloc>::type size_type;

	private:
        struct link
        {
            link* Next;
        };
        
		struct stats
		{
			Alloc Allocator;
			std::atomic<size_t> UseCount;
			link* Free;

			stats(const Alloc& alloc) :
				Allocator(alloc),
				UseCount(1),
				Free(nullptr) {}
		};

	public:
		freelist_allocator(const Alloc& alloc = Alloc()) noexcept
		{
			// Allocate the control block with the allocator, if we fit
			if constexpr (sizeof(stats) > Min && sizeof(stats) <= Max)
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

		freelist_allocator(const freelist_allocator& other) noexcept :
            m_Stats(other.m_Stats)
		{
			m_Stats->UseCount++;
		}

		freelist_allocator(freelist_allocator&& other) noexcept :
			m_Stats(other.m_Stats)
		{
			KTL_ASSERT(other.m_Stats);
			other.m_Stats = nullptr;
		}

        ~freelist_allocator()
        {
			if (m_Stats)
				decrement();
        }

		freelist_allocator& operator=(const freelist_allocator& rhs)
		{
			if (m_Stats)
				decrement();

			m_Stats = rhs.m_Stats;
			m_Stats->UseCount++;

			return *this;
		}

		freelist_allocator& operator=(freelist_allocator&& rhs)
		{
			if (m_Stats)
				decrement();

			m_Stats = rhs.m_Stats;

			rhs.m_Stats = nullptr;

			return *this;
		}

		bool operator==(const freelist_allocator& rhs) const noexcept
		{
			return m_Stats->Allocator == rhs.m_Stats->Allocator;
		}

		bool operator!=(const freelist_allocator& rhs) const noexcept
		{
			return m_Stats->Allocator != rhs.m_Stats->Allocator;
		}

#pragma region Allocation
		void* allocate(size_type n)
		{
			if (n > Min && n <= Max)
			{
				link* next = m_Stats->Free;
				if (next)
				{
					m_Stats->Free = next->Next;
					return next;
				}

				return m_Stats->Allocator.allocate(Max);
			}

			return nullptr;
		}

		void deallocate(void* p, size_type n)
		{
			if (n > Min && n <= Max && p)
			{
				link* next = reinterpret_cast<link*>(p);
				next->Next = m_Stats->Free;
				m_Stats->Free = next;
			}
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<has_construct<void, Alloc, T*, Args...>::value, void>::type
		construct(T* p, Args&&... args)
		{
			m_Stats->Allocator.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<has_destroy<Alloc, T*>::value, void>::type
		destroy(T* p)
		{
			m_Stats->Allocator.destroy(p);
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
				link* next = m_Stats->Free;
				while (next)
				{
					link* prev = next;
					next = next->Next;
					m_Stats->Allocator.deallocate(prev, Max);
				}

				if constexpr (sizeof(stats) > Min && sizeof(stats) <= Max)
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

	template<typename T, size_t Min, size_t Max, typename A>
	using type_freelist_allocator = type_allocator<T, freelist_allocator<Min, Max, A>>;
}