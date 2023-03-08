#pragma once

#include "../utility/meta.h"
#include "../utility/notomic.h"
#include "shared_fwd.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename Alloc, typename Atomic>
	class shared
	{
	private:
		static_assert(detail::has_no_value_type<Alloc>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");

		struct block
		{
			Alloc Allocator;
			Atomic UseCount;

			block() noexcept :
				Allocator(),
				UseCount(1) {}

			block(const Alloc& allocator) noexcept :
				Allocator(allocator),
				UseCount(1) {}

			block(Alloc&& allocator) noexcept :
				Allocator(std::move(allocator)),
				UseCount(1) {}
		};

	public:
		typedef typename detail::get_size_type<Alloc>::type size_type;

		shared() noexcept :
			m_Block(new block) {}

		explicit shared(const Alloc& alloc) noexcept :
			m_Block(new block(alloc)) {}

		explicit shared(Alloc&& alloc) noexcept :
			m_Block(new block(std::move(alloc))) {}

		shared(const shared& other) noexcept :
			m_Block(other.m_Block)
		{
			increment();
		}

		shared(shared&& other) noexcept :
			m_Block(std::move(other.m_Block))
		{
			other.m_Block = nullptr;
		}

		~shared()
		{
			decrement();
		}

		shared& operator=(const shared& rhs) noexcept
		{
			decrement();

			m_Block = rhs.m_Block;

			increment();

			return *this;
		}

		shared& operator=(shared&& rhs) noexcept
		{
			decrement();

			m_Block = rhs.m_Block;

			rhs.m_Block = nullptr;

			return *this;
		}

		bool operator==(const shared& rhs) const noexcept
		{
			return m_Block == rhs.m_Block && m_Block->Allocator == rhs.m_Block->Allocator;
		}

		bool operator!=(const shared& rhs) const noexcept
		{
			return m_Block != rhs.m_Block || m_Block->Allocator != rhs.m_Block->Allocator;
		}

#pragma region Allocation
		void* allocate(size_t n)
		{
			return m_Block->Allocator.allocate(n);
		}

		void deallocate(void* p, size_t n)
		{
			m_Block->Allocator.deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct<void, Alloc, T*, Args...>::value, void>::type
		construct(T* p, Args&&... args)
		{
			m_Block->Allocator.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy<Alloc, T*>::value, void>::type
		destroy(T* p)
		{
			m_Block->Allocator.destroy(p);
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size<A>::value, size_type>::type
		max_size() const noexcept
		{
			return m_Block->Allocator.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns<A>::value, bool>::type
		owns(void* p) const
		{
			return m_Block->Allocator.owns(p);
		}
#pragma endregion

		Alloc& get_allocator()
		{
			return m_Block->Allocator;
		}

		const Alloc& get_allocator() const
		{
			return m_Block->Allocator;
		}

	private:
		void increment()
		{
			if (!m_Block) return;

			m_Block->UseCount++;
		}

		void decrement()
		{
			if (!m_Block) return;

			if (m_Block->UseCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
				delete m_Block;
		}

	private:
		block* m_Block;
	};
}