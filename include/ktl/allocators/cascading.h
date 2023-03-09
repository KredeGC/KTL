#pragma once

#include "../utility/assert.h"
#include "../utility/meta.h"
#include "../utility/notomic.h"
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
			Alloc Allocator;
			size_type Allocations = 0;
			node* Next = nullptr;
		};

	public:
		cascading() noexcept :
			m_Node(nullptr) {}

		cascading(const cascading&) noexcept = delete;

		cascading(cascading&& other) noexcept :
			m_Node(other.m_Node)
		{
			other.m_Node = nullptr;
		}

		~cascading()
		{
			release();
		}

		cascading& operator=(const cascading&) noexcept = delete;

		cascading& operator=(cascading&& rhs) noexcept
		{
			release();

			m_Node = rhs.m_Node;

			rhs.m_Node = nullptr;

			return *this;
		}

		bool operator==(const cascading& rhs) const noexcept
		{
			return m_Node == rhs.m_Node;
		}

		bool operator!=(const cascading& rhs) const noexcept
		{
			return m_Node != rhs.m_Node;
		}

#pragma region Allocation
		void* allocate(size_type n)
		{
			// Add an initial allocator
			if (!m_Node)
				m_Node = new node;

			if constexpr (detail::has_max_size<Alloc>::value)
			{
				if (n > m_Node->Allocator.max_size())
					return nullptr;
			}

			void* p = m_Node->Allocator.allocate(n);

			// If the allocator was unable to allocate it, create a new one
			if (p == nullptr)
			{
				node* next = m_Node;
				m_Node = new node;
				m_Node->Next = next;

				p = m_Node->Allocator.allocate(n);
			}

			if (p)
				m_Node->Allocations++;

			return p;
		}

		void deallocate(void* p, size_type n) noexcept
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
						delete next;
					}

					break;
				}

				prev = next;
				next = next->Next;
			}
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args)
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

			::new(p) T(std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p)
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

			p->~T();
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const noexcept
		{
			return m_Node->Allocator.max_size();
		}

		bool owns(void* p) const
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
		void release() noexcept
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

				delete current;
			}
		}

	private:
		node* m_Node;
	};
}