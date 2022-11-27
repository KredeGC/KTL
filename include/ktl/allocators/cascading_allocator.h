#pragma once

#include "../utility/assert_utility.h"
#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <atomic>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename Alloc>
	class cascading_allocator
	{
	private:
		static_assert(has_no_value_type<Alloc>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(has_owns<Alloc>::value, "The allocator is required to have an 'owns(void*)' method");

	public:
		typedef typename get_size_type<Alloc>::type size_type;

	private:
		struct node
		{
			Alloc Allocator;
			size_type Allocations = 0;
			node* Next = nullptr;
		};

		struct block
		{
			std::atomic<size_t> UseCount;
			node* Node;

			block() noexcept :
				UseCount(1),
				Node(nullptr) {}
		};

	public:
		cascading_allocator() noexcept
		{
			m_Block = new block;
		}

		cascading_allocator(const cascading_allocator& other) noexcept :
			m_Block(other.m_Block)
		{
			KTL_ASSERT(other.m_Block);
			m_Block->UseCount++;
		}

		cascading_allocator(cascading_allocator&& other) noexcept :
			m_Block(other.m_Block)
		{
			KTL_ASSERT(other.m_Block);
			other.m_Block = nullptr;
		}

		~cascading_allocator()
		{
			if (m_Block)
				decrement();
		}

		cascading_allocator& operator=(const cascading_allocator& rhs) noexcept
		{
			if (m_Block)
				decrement();

			m_Block = rhs.m_Block;
			m_Block->UseCount++;

			return *this;
		}

		cascading_allocator& operator=(cascading_allocator&& rhs) noexcept
		{
			if (m_Block)
				decrement();

			m_Block = rhs.m_Block;

			rhs.m_Block = nullptr;

			return *this;
		}

		bool operator==(const cascading_allocator& rhs) const noexcept
		{
			return m_Block == rhs.m_Block;
		}

		bool operator!=(const cascading_allocator& rhs) const noexcept
		{
			return m_Block != rhs.m_Block;
		}

#pragma region Allocation
		void* allocate(size_type n)
		{
			// Add an initial allocator
			if (!m_Block->Node)
				m_Block->Node = new node;

			if constexpr (has_max_size<Alloc>::value)
			{
				if (n > m_Block->Node->Allocator.max_size())
					return nullptr;
			}

			void* p = m_Block->Node->Allocator.allocate(n);

			// If the allocator was unable to allocate it, create a new one
			if (p == nullptr)
			{
				node* next = m_Block->Node;
				m_Block->Node = new node;
				m_Block->Node->Next = next;

				p = m_Block->Node->Allocator.allocate(n);
			}

			if (p)
				m_Block->Node->Allocations++;

			return p;
		}

		void deallocate(void* p, size_type n) noexcept
		{
			node* prev = nullptr;
			node* next = m_Block->Node;
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
		typename std::enable_if<has_construct<void, Alloc, T*, Args...>::value, void>::type
		construct(T* p, Args&&... args)
		{
            node* next = m_Block->Node;
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
		typename std::enable_if<has_destroy<Alloc, T*>::value, void>::type
		destroy(T* p)
		{
			node* next = m_Block->Node;
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
		typename std::enable_if<has_max_size<A>::value, size_type>::type
		max_size() const noexcept
		{
			return m_Block->Node->Allocator.max_size();
		}

		bool owns(void* p)
		{
			node* next = m_Block->Node;
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
		void decrement()
		{
			if (m_Block->UseCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				node* next = m_Block->Node;
				while (next)
				{
					// Assert that we only have a single allocator left
					// Otherwise someone forgot to deallocate memory
					// This isn't a hard-error though
					KTL_ASSERT(next == m_Block->Node);

					node* current = next;

					next = current->Next;

					delete current;
				}
			}
		}

		block* m_Block;
	};

	template<typename T, typename Alloc>
	using type_cascading_allocator = type_allocator<T, cascading_allocator<Alloc>>;
}