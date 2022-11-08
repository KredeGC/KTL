#pragma once

#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Min, size_t Max, size_t Batch, size_t Threshold, typename Alloc>
	class freelist_allocator
	{
	private:
		static_assert(Threshold % Batch == 0, "Threshold must be a multiple of Batch count");
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
			size_t Count;

			stats(const Alloc& alloc) :
				Allocator(alloc),
				Free(nullptr),
				Count(0) {}
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
				if (!m_Stats->Free && m_Stats->Count < Threshold)
				{
					for (size_t i = 0; i < Batch; i++)
					{
						link* next = reinterpret_cast<link*>(m_Stats->Allocator.allocate(Max));
						if (!next)
							break;

						next->Next = m_Stats->Free;
						m_Stats->Free = next;
						m_Stats->Count++;
					}
				}

				link* next = m_Stats->Free;
				if (next)
					m_Stats->Free = next->Next;
				return next;
			}

			return nullptr;
		}

		void deallocate(void* p, size_type n)
		{
			if (n > Min && n <= Max && m_Stats->Count < Threshold)
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

	template<size_t Min1, size_t Max1, size_t Batch1, size_t Threshold1, typename A, size_t Min2, size_t Max2, size_t Batch2, size_t Threshold2, typename U>
	bool operator==(const freelist_allocator<Min1, Min2, Batch1, Threshold1, A>& lhs, const freelist_allocator<Min2, Max2, Batch2, Threshold2, U>& rhs) noexcept
	{
		return lhs.m_Stats->Allocator == rhs.m_Stats->Allocator;
	}

	template<size_t Min1, size_t Max1, size_t Batch1, size_t Threshold1, typename A, size_t Min2, size_t Max2, size_t Batch2, size_t Threshold2, typename U>
	bool operator!=(const freelist_allocator<Min1, Min2, Batch1, Threshold1, A>& lhs, const freelist_allocator<Min2, Max2, Batch2, Threshold2, U>& rhs) noexcept
	{
		return lhs.m_Stats->Allocator != rhs.m_Stats->Allocator;
	}

	template<typename T, size_t Min, size_t Max, size_t Batch, size_t Threshold, typename A>
	using type_freelist_allocator = type_allocator<T, freelist_allocator<Min, Max, Batch, Threshold, A>>;
}