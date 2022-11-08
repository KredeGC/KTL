#pragma once

#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Min, size_t Max, typename Alloc>
	class freelist_allocator
	{
	private:
		static_assert(has_value_type<Alloc>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");

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
			link* Free;

			stats(const Alloc& alloc) :
				Allocator(alloc),
				Free(nullptr) {}
		};

	public:
		freelist_allocator(const Alloc& alloc = Alloc()) noexcept :
            m_Stats(std::make_shared<stats>(alloc)) {}

		freelist_allocator(const freelist_allocator& other) noexcept :
            m_Stats(other.m_Stats) {}

        ~freelist_allocator()
        {
            if (m_Stats.use_count() == 1)
            {
                link* next = m_Stats->Free;
                while (next)
                {
                    link* prev = next;
                    next = next->Next;
					m_Stats->Allocator.deallocate(prev, Max);
                }
            }
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
        std::shared_ptr<stats> m_Stats;
	};

	template<size_t Min1, size_t Max1, typename A, size_t Min2, size_t Max2, typename U>
	bool operator==(const freelist_allocator<Min1, Min2, A>& lhs, const freelist_allocator<Min2, Max2, U>& rhs) noexcept
	{
		return lhs.m_Stats->Allocator == rhs.m_Stats->Allocator;
	}

	template<size_t Min1, size_t Max1, typename A, size_t Min2, size_t Max2, typename U>
	bool operator!=(const freelist_allocator<Min1, Min2, A>& lhs, const freelist_allocator<Min2, Max2, U>& rhs) noexcept
	{
		return lhs.m_Stats->Allocator != rhs.m_Stats->Allocator;
	}

	template<typename T, size_t Min, size_t Max, typename A>
	using type_freelist_allocator = type_allocator<T, freelist_allocator<Min, Max, A>>;
}