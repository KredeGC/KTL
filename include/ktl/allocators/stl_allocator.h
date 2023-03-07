#pragma once

#include "../utility/meta.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename Alloc>
	class stl_allocator
	{
	private:
		static_assert(detail::has_no_value_type<Alloc>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");

		struct block
		{
			Alloc Allocator;
			size_t UseCount;

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

		stl_allocator() noexcept :
			m_Block(new block) {}

		explicit stl_allocator(const Alloc& alloc) noexcept :
			m_Block(new block(alloc)) {}

		explicit stl_allocator(Alloc&& alloc) noexcept :
			m_Block(new block(std::move(alloc))) {}

		stl_allocator(const stl_allocator& other) noexcept :
			m_Block(other.m_Block)
		{
			increment();
		}

		stl_allocator(stl_allocator&& other) noexcept :
			m_Block(std::move(other.m_Block))
		{
			other.m_Block = nullptr;
		}

		~stl_allocator()
		{
			decrement();
		}

		stl_allocator& operator=(const stl_allocator& rhs) noexcept
		{
			decrement();

			m_Block = rhs.m_Block;

			increment();

			return *this;
		}

		stl_allocator& operator=(stl_allocator&& rhs) noexcept
		{
			decrement();

			m_Block = rhs.m_Block;

			rhs.m_Block = nullptr;

			return *this;
		}

		bool operator==(const stl_allocator& rhs) const noexcept
		{
			return m_Block == m_Block;
		}

		bool operator!=(const stl_allocator& rhs) const noexcept
		{
			return m_Block != m_Block;
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

			size_t count = --m_Block->UseCount;

			if (count == 0)
				delete m_Block;
		}

	private:
		block* m_Block;
	};
}