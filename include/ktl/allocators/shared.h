#pragma once

#include "../utility/aligned_malloc.h"
#include "../utility/alignment.h"
#include "../utility/empty_base.h"
#include "../utility/meta.h"
#include "shared_fwd.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename Alloc>
	class shared
	{
	private:
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");

		struct block
		{
			KTL_EMPTY_BASE Alloc Allocator;
			detail::get_size_type_t<Alloc> UseCount;

			template<typename... Args,
				typename = std::enable_if_t<
				std::is_constructible_v<Alloc, Args...>>>
			block(Args&&... alloc)
				noexcept(std::is_nothrow_constructible_v<Alloc, Args...>) :
				Allocator(std::forward<Args>(alloc)...),
				UseCount(1) {}
		};

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

		template<typename A = Alloc>
		shared()
			noexcept(std::is_nothrow_default_constructible_v<block>) :
			m_Block(detail::aligned_new<block>(detail::ALIGNMENT)) {}

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			std::is_constructible_v<Alloc, Args...>>>
		explicit shared(Args&&... alloc)
			noexcept(std::is_nothrow_constructible_v<block, Args...>) :
			m_Block(detail::aligned_new<block>(detail::ALIGNMENT, std::forward<Args>(alloc)...)) {}

		shared(const shared& other) noexcept :
			m_Block(other.m_Block)
		{
			increment();
		}

		shared(shared&& other) noexcept :
			m_Block(other.m_Block)
		{
			other.m_Block = nullptr;
		}

		~shared()
		{
			decrement();
		}

		shared& operator=(const shared& rhs)
			noexcept(noexcept(decrement()))
		{
			decrement();

			m_Block = rhs.m_Block;

			increment();

			return *this;
		}

		shared& operator=(shared&& rhs)
			noexcept(noexcept(decrement()))
		{
			decrement();

			m_Block = rhs.m_Block;

			rhs.m_Block = nullptr;

			return *this;
		}

		bool operator==(const shared& rhs) const
			noexcept(detail::has_nothrow_equal_v<Alloc>)
		{
			return m_Block == rhs.m_Block && m_Block->Allocator == rhs.m_Block->Allocator;
		}

		bool operator!=(const shared& rhs) const
			noexcept(detail::has_nothrow_not_equal_v<Alloc>)
		{
			return m_Block != rhs.m_Block || m_Block->Allocator != rhs.m_Block->Allocator;
		}

#pragma region Allocation
		void* allocate(size_t n)
			noexcept(detail::has_nothrow_allocate_v<Alloc>)
		{
			return m_Block->Allocator.allocate(n);
		}

		void deallocate(void* p, size_t n)
			noexcept(detail::has_nothrow_deallocate_v<Alloc>)
		{
			m_Block->Allocator.deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args)
			noexcept(detail::has_nothrow_construct_v<Alloc, T*, Args...>)
		{
			m_Block->Allocator.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p)
			noexcept(detail::has_nothrow_destroy_v<Alloc, T*>)
		{
			m_Block->Allocator.destroy(p);
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const
			noexcept(detail::has_nothrow_max_size_v<A>)
		{
			return m_Block->Allocator.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
			noexcept(detail::has_nothrow_owns_v<A>)
		{
			return m_Block->Allocator.owns(p);
		}
#pragma endregion

		Alloc& get_allocator() noexcept
		{
			return m_Block->Allocator;
		}

		const Alloc& get_allocator() const noexcept
		{
			return m_Block->Allocator;
		}

	private:
		void increment() noexcept
		{
			if (!m_Block) return;

			m_Block->UseCount++;
		}

		void decrement()
			noexcept(std::is_nothrow_destructible_v<Alloc>)
		{
			if (!m_Block) return;

			if (--m_Block->UseCount == 0)
				detail::aligned_delete(m_Block);
		}

	private:
		block* m_Block;
	};
}