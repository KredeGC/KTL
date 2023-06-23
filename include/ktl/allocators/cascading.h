#pragma once

#include "../utility/aligned_malloc.h"
#include "../utility/alignment.h"
#include "../utility/assert.h"
#include "../utility/empty_base.h"
#include "../utility/meta.h"
#include "cascading_fwd.h"
#include "type_allocator.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	/**
	 * @brief An allocator which owns multiple instances of a given sub allocator.
	 * When allocating it will attempt to use the first available allocator.
	 * Upon failure it will instead create a new instance and allocate from it.
	 * @note Deallocation can take O(n) time as it may have to traverse multiple allocator instances to find the right one.
	 * The allocator type must be default-constructible, which means any variation of stack_allocator can't be used.
	 * The allocator type must also have an owns(*ptr) method, which means any variation of mallocator can't be used.
	 * @tparam Alloc The allocator type to create instances from
     * @tparam Atomic The atomic type to use for reference counting. Threading is not implemented, so this defaults to ktl::notomic<size_t>
	*/
	template<typename Alloc>
	class cascading
	{
	private:
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(detail::has_owns_v<Alloc>, "The allocator is required to have an 'owns(void*)' method");

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

	private:
		struct node
		{
			KTL_EMPTY_BASE Alloc Allocator;
			size_type Allocations = 0;
			node* Next = nullptr;
		};

	public:
		cascading() noexcept :
			m_Node(nullptr) {}

		cascading(const cascading&) = delete;

		cascading(cascading&& other)
			noexcept(noexcept(node(std::declval<node&&>()))) :
			m_Node(std::move(other.m_Node))
		{
			other.m_Node = nullptr;
		}

		~cascading()
		{
			release();
		}

		cascading& operator=(const cascading&) = delete;

		cascading& operator=(cascading&& rhs)
			noexcept(noexcept(m_Node = std::move(rhs.m_Node)))
		{
			release();

			m_Node = std::move(rhs.m_Node);

			rhs.m_Node = nullptr;

			return *this;
		}

		bool operator==(const cascading& rhs) const
			noexcept(noexcept(m_Node == rhs.m_Node))
		{
			return m_Node == rhs.m_Node;
		}

		bool operator!=(const cascading& rhs) const
			noexcept(noexcept(m_Node != rhs.m_Node))
		{
			return m_Node != rhs.m_Node;
		}

#pragma region Allocation
		/**
		 * @brief Attempts to allocate a chunk of memory defined by @p n
		 * @note May create a new instance of the underlying allocator and attempt to allocate using it
		 * @param n The amount of bytes to allocate memory for
		 * @return A location in memory that is at least @p n bytes big or nullptr if it could not be allocated
		*/
		void* allocate(size_type n) noexcept(
			noexcept(detail::aligned_new<node>(detail::ALIGNMENT)) &&
			noexcept(std::declval<Alloc&>().allocate(n)) &&
			(!detail::has_max_size_v<Alloc> || noexcept(std::declval<Alloc&>().max_size())))
		{
			// Add an initial allocator
			if (!m_Node)
				m_Node = detail::aligned_new<node>(detail::ALIGNMENT);

			if constexpr (detail::has_max_size_v<Alloc>)
			{
				if (n > m_Node->Allocator.max_size())
					return nullptr;
			}

			void* p = m_Node->Allocator.allocate(n);

			// If the allocator was unable to allocate it, create a new one
			if (p == nullptr)
			{
				node* next = m_Node;

				m_Node = detail::aligned_new<node>(detail::ALIGNMENT);
				m_Node->Next = next;

				p = m_Node->Allocator.allocate(n);
			}

			if (p)
				m_Node->Allocations++;

			return p;
		}

		/**
		 * @brief Attempts to deallocate the memory at location @p p
		 * @note Deallocation can take O(n) time as it may have to traverse all allocator instances
		 * @param p The location in memory to deallocate
		 * @param n The size that was initially allocated
		*/
		void deallocate(void* p, size_type n)
			noexcept(noexcept(detail::aligned_delete(std::declval<node*>())) && noexcept(std::declval<Alloc&>().owns(p)) && noexcept(std::declval<Alloc&>().deallocate(p, n)))
		{
			KTL_ASSERT(p != nullptr);

			node* prev = nullptr;
			node* next = m_Node;
			while (next)
			{
				if (next->Allocator.owns(p))
				{
					next->Allocator.deallocate(p, n);

					// If this allocator holds no allocations then delete it
					// Unless it's the main one, in which case keep it
					if (--next->Allocations == 0 && prev)
					{
						prev->Next = next->Next;

						detail::aligned_delete(next);
					}

					break;
				}

				prev = next;
				next = next->Next;
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
			noexcept(noexcept(std::declval<Alloc&>().owns(p)) && detail::has_noexcept_construct_v<Alloc, T*, Args...>)
		{
            node* next = m_Node;
			while (next)
			{
				if (next->Allocator.owns(p))
                {
                    next->Allocator.construct(p, std::forward<Args>(args)...);
					return;
                }

				next = next->Next;
			}

			// If we ever get to this point, something has gone wrong with the internal allocators
			KTL_ASSERT(false);

			::new(p) T(std::forward<Args>(args)...);
		}

		/**
		 * @brief Destructs an object of T at the given location
		 * @note Only defined if the underlying allocator defines it
		 * @param p The location of the object in memory
		*/
		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p)
			noexcept(noexcept(std::declval<Alloc&>().owns(p)) && detail::has_noexcept_destroy_v<Alloc, T*>)
		{
			node* next = m_Node;
			while (next)
			{
				if (next->Allocator.owns(p))
                {
                    next->Allocator.destroy(p);
					return;
                }

				next = next->Next;
			}

			// If we ever get to this point, something has gone wrong with the internal allocators
			KTL_ASSERT(false);

			p->~T();
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
			noexcept(noexcept(std::declval<A&>().max_size()))
		{
			return m_Node->Allocator.max_size();
		}

		/**
		 * @brief Returns whether or not the allocator owns the given location in memory
		 * @param p The location of the object in memory
		 * @return Whether the allocator owns @p p
		*/
		bool owns(void* p) const
			noexcept(noexcept(std::declval<Alloc&>().owns(p)))
		{
			node* next = m_Node;
			while (next)
			{
				if (next->Allocator.owns(p))
					return true;

				next = next->Next;
			}

			return false;
		}
#pragma endregion

	private:
		void release()
			noexcept(noexcept(detail::aligned_delete(m_Node)))
		{
			node* next = m_Node;
			while (next)
			{
				// Assert that we only have a single allocator left
				// Otherwise someone forgot to deallocate memory
				// This isn't a hard-error though
				KTL_ASSERT(next == m_Node);

				node* current = next;

				next = current->Next;

				detail::aligned_delete(current);
			}
		}

	private:
		node* m_Node;
	};
}