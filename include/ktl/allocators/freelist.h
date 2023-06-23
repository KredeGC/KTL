#pragma once

#include "../utility/assert.h"
#include "../utility/empty_base.h"
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
		template<typename A = Alloc>
		freelist()
			noexcept(std::is_nothrow_default_constructible_v<A>) :
			m_Alloc(),
			m_Free(nullptr) {}

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			std::is_constructible_v<Alloc, Args...>>>
		explicit freelist(Args&&... args)
			noexcept(std::is_nothrow_constructible_v<Alloc, Args...>) :
			m_Alloc(std::forward<Args>(args)...),
			m_Free(nullptr) {}

		freelist(const freelist&) = delete;

		freelist(freelist&& other)
			noexcept(std::is_nothrow_move_constructible<Alloc>) :
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

		freelist& operator=(const freelist&) = delete;

		freelist& operator=(freelist&& rhs)
			noexcept(std::is_nothrow_move_constructible_v<Alloc>)
		{
			release();

			m_Alloc = std::move(rhs.m_Alloc);
			m_Free = rhs.m_Free;

			// Moving raw allocators in use is undefined
			KTL_ASSERT(m_Alloc == rhs.m_Alloc || rhs.m_Free == nullptr);

			rhs.m_Free = nullptr;

			return *this;
		}

		bool operator==(const freelist& rhs) const
			noexcept(noexcept(m_Alloc == rhs.m_Alloc))
		{
			return m_Alloc == rhs.m_Alloc && m_Free == rhs.m_Free;
		}

		bool operator!=(const freelist& rhs) const
			noexcept(noexcept(m_Alloc != rhs.m_Alloc))
		{
			return m_Alloc != rhs.m_Alloc || m_Free != rhs.m_Free;
		}

#pragma region Allocation
		/**
		 * @brief Attempts to allocate a chunk of memory defined by @p n.
		 * Will use previous allocations that were meant to be deallocated.
		 * @note If @p n is not within Min and Max, this function will return nullptr
		 * @param n The amount of bytes to allocate memory for
		 * @return A location in memory that is at least @p n bytes big or nullptr if it could not be allocated
		*/
		void* allocate(size_type n)
			noexcept(detail::has_nothrow_allocate_v<Alloc>)
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

		/**
		 * @brief Attempts to deallocate the memory at location @p p
		 * @note Will not deallocate the memory, but instead tie it to a linked list for later reuse
		 * @param p The location in memory to deallocate
		 * @param n The size that was initially allocated
		*/
		void deallocate(void* p, size_type n) noexcept
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
		/**
		 * @brief Constructs an object of T with the given @p ...args at the given location
		 * @note Only defined if the underlying allocator defines it
		 * @tparam ...Args The types of the arguments
		 * @param p The location of the object in memory
		 * @param ...args A range of arguments to use to construct the object
		*/
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args)
			noexcept(detail::has_nothrow_construct_v<Alloc, T*, Args...>)
		{
			m_Alloc.construct(p, std::forward<Args>(args)...);
		}

		/**
		 * @brief Destructs an object of T at the given location
		 * @note Only defined if the underlying allocator defines it
		 * @param p The location of the object in memory
		*/
		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p)
			noexcept(detail::has_nothrow_destroy_v<Alloc, T*>)
		{
			m_Alloc.destroy(p);
		}
#pragma endregion

#pragma region Utility
		/**
		 * @brief Returns the maximum size that an allocation can be
		 * @note Only defined if the underlying allocator defines it
		 * @return The maximum size an allocation may be
		*/
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const
			noexcept(detail::has_nothrow_max_size_v<A>)
		{
			return m_Alloc.max_size();
		}

		/**
		 * @brief Returns whether or not the allocator owns the given location in memory
		 * @note Only defined if the underlying allocator defines it
		 * @param p The location of the object in memory
		 * @return Whether the allocator owns @p p
		*/
		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
			noexcept(detail::has_nothrow_owns_v<A>)
		{
			return m_Alloc.owns(p);
		}
#pragma endregion

		/**
		 * @brief Returns a reference to the underlying allocator
		 * @return The allocator
		*/
		Alloc& get_allocator() noexcept
		{
			return m_Alloc;
		}

		/**
		 * @brief Returns a const reference to the underlying allocator
		 * @return The allocator
		*/
		const Alloc& get_allocator() const noexcept
		{
			return m_Alloc;
		}

	private:
		void release()
			noexcept(detail::has_nothrow_deallocate_v<Alloc>)
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
		KTL_EMPTY_BASE Alloc m_Alloc;
		link* m_Free;
	};
}