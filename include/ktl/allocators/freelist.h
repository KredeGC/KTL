#pragma once

#include "../utility/assert.h"
#include "../utility/meta.h"
#include "freelist_fwd.h"
#include "type_allocator.h"

#include <cstddef>
#include <memory>
#include <type_traits>

namespace ktl
{
	/**
	 * @brief An allocator which allocates using its underlying allocator.
	 * Only allocates if the requested size is within the @p Min and @p Max range
	 * When deallocating it chains the memory in a linked list, which can then be reused.
	 * @note The memory is only completely dealloacated when the allocator is destroyed.
	 * The value of @p Max must be at least the size of a pointer, otherwise it cannot chain deallocations.
	 * @tparam Alloc The allocator to wrap around
	*/
	template<size_t Min, size_t Max, typename Alloc>
	class freelist
	{
	private:
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(Max >= sizeof(void*), "The freelist allocator requires a Max of at least the size of a pointer");

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

	private:
        struct link
        {
            link* Next;
        };
        
	public:
		freelist() noexcept :
			m_Alloc(),
			m_Free(nullptr) {}

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			detail::can_construct_v<Alloc, Args...>>>
		explicit freelist(Args&&... args) noexcept :
			m_Alloc(std::forward<Args>(args)...),
			m_Free(nullptr) {}

		freelist(const freelist&) noexcept = delete;

		freelist(freelist&& other) noexcept :
			m_Alloc(std::move(other.m_Alloc)),
			m_Free(other.m_Free)
		{
			// Moving raw allocators in use is undefined
			KTL_ASSERT(m_Alloc == other.m_Alloc || other.m_Free == nullptr);

			other.m_Free = nullptr;
		}

        ~freelist()
        {
			release();
        }

		freelist& operator=(const freelist&) noexcept = delete;

		freelist& operator=(freelist&& rhs) noexcept
		{
			release();

			m_Alloc = std::move(rhs.m_Alloc);
			m_Free = rhs.m_Free;

			// Moving raw allocators in use is undefined
			KTL_ASSERT(m_Alloc == rhs.m_Alloc || rhs.m_Free == nullptr);

			rhs.m_Free = nullptr;

			return *this;
		}

		bool operator==(const freelist& rhs) const noexcept
		{
			return m_Alloc == rhs.m_Alloc && m_Free == rhs.m_Free;
		}

		bool operator!=(const freelist& rhs) const noexcept
		{
			return m_Alloc != rhs.m_Alloc || m_Free != rhs.m_Free;
		}

#pragma region Allocation
		void* allocate(size_type n)
		{
			if (n > Min && n <= Max)
			{
				link* next = m_Free;
				if (next)
				{
					m_Free = next->Next;
					return next;
				}

				return m_Alloc.allocate(Max);
			}

			return nullptr;
		}

		void deallocate(void* p, size_type n)
		{
			KTL_ASSERT(p != nullptr);

			if (n > Min && n <= Max && p)
			{
				link* next = reinterpret_cast<link*>(p);
				next->Next = m_Free;
				m_Free = next;
			}
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args)
		{
			m_Alloc.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p)
		{
			m_Alloc.destroy(p);
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const noexcept
		{
			return m_Alloc.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
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
		void release() noexcept
		{
			link* next = m_Free;
			while (next)
			{
				link* prev = next;
				next = next->Next;
				m_Alloc.deallocate(prev, Max);
			}
		}

	private:
		Alloc m_Alloc;
		link* m_Free;
	};
}