#pragma once

#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Min, size_t Max, size_t Threshold, typename Alloc>
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
            size_t Size;
        };
        
        struct stats
        {
            link* Next = nullptr;
            size_t Count = 0;
        };

	public:
		freelist_allocator(const Alloc& alloc = Alloc()) noexcept :
            m_Alloc(alloc),
            m_Stats(std::make_shared<stats>()) {}

		freelist_allocator(const freelist_allocator& other) noexcept :
            m_Alloc(other.m_Alloc),
            m_Stats(other.m_Stats) {}

        ~freelist_allocator()
        {
            if (m_Stats.use_count() == 1)
            {
                link* next = m_Stats->Next;
                while (next)
                {
                    link* prev = next;
                    next = next->Next;
                    m_Alloc.deallocate(prev, prev->Size);
                }
            }
        }

#pragma region Allocation
		void* allocate(size_type n)
		{
            if (m_Stats->Next)
            {
                link* next = m_Stats->Next;
                m_Stats->Next = next->Next;
                m_Stats->Count--;
                return next;
            }
			return m_Alloc.allocate(n);
		}

		void deallocate(void* p, size_type n)
		{
            if (m_Stats->Count < Threshold && n > Min && n < Max)
            {
                link* next = reinterpret_cast<link*>(p);
                next->Next = m_Stats->Next;
                next->Size = n;
                m_Stats->Next = next;
                m_Stats->Count++;
            }
            else
            {
			    m_Alloc.deallocate(p, n);
            }
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<has_construct<void, Alloc, T*, Args...>::value, void>::type
		construct(T* p, Args&&... args)
		{
			m_Alloc.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<has_destroy<Alloc, T*>::value, void>::type
		destroy(T* p)
		{
			m_Alloc.destroy(p);
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

	template<size_t Min1, size_t Max1, size_t Threshold1, typename A, size_t Min2, size_t Max2, size_t Threshold2, typename U>
	bool operator==(const freelist_allocator<Min1, Min2, Threshold1, A>& lhs, const freelist_allocator<Min2, Max2, Threshold2, U>& rhs) noexcept
	{
		return &lhs == &rhs;
	}

	template<size_t Min1, size_t Max1, size_t Threshold1, typename A, size_t Min2, size_t Max2, size_t Threshold2, typename U>
	bool operator!=(const freelist_allocator<Min1, Min2, Threshold1, A>& lhs, const freelist_allocator<Min2, Max2, Threshold2, U>& rhs) noexcept
	{
		return &lhs != &rhs;
	}

	template<typename T, size_t Min, size_t Max, size_t Threshold, typename A>
	using type_freelist_allocator = type_allocator<T, freelist_allocator<Min, Max, Threshold, A>>;
}