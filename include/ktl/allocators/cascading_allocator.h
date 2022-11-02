#pragma once

#include "type_allocator.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename Alloc>
	class cascading_allocator
	{
	private:
		static_assert(has_value_type<Alloc>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(has_owns<Alloc>::value, "The allocator is required to have an 'owns(void*)' method");

	public:
		typedef typename get_size_type<Alloc>::type size_type;

	private:
		struct node
		{
			Alloc Allocator;
			size_t Allocations = 0;
			node* Next = nullptr;
		};

		struct block
		{
			node* Node = nullptr;
		};

	public:
		cascading_allocator() noexcept
			: m_Block(std::make_shared<block>()) {}

		cascading_allocator(const cascading_allocator& other) noexcept :
			m_Block(other.m_Block) {}

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

			// If the allocator was unable to allocate it, try without
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

					if (--next->Allocations == 0)
					{
						if (prev)
							prev->Next = next->Next;
						else
							m_Block->Node = next->Next;

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
			m_Block->Node->Allocator.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<has_destroy<Alloc, T*>::value, void>::type
		destroy(T* p)
		{
			m_Block->Node->Allocator.destroy(p);
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
		std::shared_ptr<block> m_Block;
	};

	template<typename A, typename U>
	bool operator==(const cascading_allocator<A>& lhs, const cascading_allocator<U>& rhs) noexcept
	{
		return &lhs == &rhs;
	}

	template<typename A, typename U>
	bool operator!=(const cascading_allocator<A>& lhs, const cascading_allocator<U>& rhs) noexcept
	{
		return &lhs != &rhs;
	}

	template<typename T, typename Alloc>
	using type_cascading_allocator = type_allocator<T, cascading_allocator<Alloc>>;
}