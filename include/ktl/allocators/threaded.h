#pragma once

#include "../utility/aligned_malloc.h"
#include "../utility/alignment.h"
#include "../utility/empty_base.h"
#include "../utility/meta.h"
#include "threaded_fwd.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <type_traits>

namespace ktl
{
	template<typename Alloc>
	class threaded
	{
	private:
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");

		struct block
		{
			KTL_EMPTY_BASE Alloc Allocator;
			std::atomic<detail::get_size_type_t<Alloc>> UseCount;
			std::mutex Lock;

			template<typename... Args,
				typename = std::enable_if_t<
				detail::can_construct_v<Alloc, Args...>>>
			block(Args&&... alloc)
				noexcept(noexcept(Alloc(std::declval<Args>()...))) :
				Allocator(std::forward<Args>(alloc)...),
				UseCount(1),
				Lock() {}
		};

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

		threaded()
			noexcept(noexcept(block())) :
			m_Block(detail::aligned_new<block>(detail::ALIGNMENT)) {}

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			detail::can_construct_v<Alloc, Args...>>>
		explicit threaded(Args&&... alloc)
			noexcept(noexcept(block(std::declval<Args>()...))) :
			m_Block(detail::aligned_new<block>(detail::ALIGNMENT, std::forward<Args>(alloc)...)) {}

		threaded(const threaded& other) noexcept :
			m_Block(other.m_Block)
		{
			increment();
		}

		threaded(threaded&& other) noexcept :
			m_Block(other.m_Block)
		{
			other.m_Block = nullptr;
		}

		~threaded()
		{
			decrement();
		}

		threaded& operator=(const threaded& rhs)
			noexcept(noexcept(decrement()))
		{
			decrement();

			m_Block = rhs.m_Block;

			increment();

			return *this;
		}

		threaded& operator=(threaded&& rhs)
			noexcept(noexcept(decrement()))
		{
			decrement();

			m_Block = rhs.m_Block;

			rhs.m_Block = nullptr;

			return *this;
		}

		bool operator==(const threaded& rhs) const
			noexcept(noexcept(m_Block->Allocator == rhs.m_Block->Allocator))
		{
			return m_Block == rhs.m_Block && m_Block->Allocator == rhs.m_Block->Allocator;
		}

		bool operator!=(const threaded& rhs) const
			noexcept(noexcept(m_Block->Allocator != rhs.m_Block->Allocator))
		{
			return m_Block != rhs.m_Block || m_Block->Allocator != rhs.m_Block->Allocator;
		}

#pragma region Allocation
		void* allocate(size_t n) // Lock cannot be noexcept
		{
			std::lock_guard<std::mutex> lock(m_Block->Lock);

			return m_Block->Allocator.allocate(n);
		}

		void deallocate(void* p, size_t n) // Lock cannot be noexcept
		{
			std::lock_guard<std::mutex> lock(m_Block->Lock);

			m_Block->Allocator.deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args) // Lock cannot be noexcept
		{
			std::lock_guard<std::mutex> lock(m_Block->Lock);

			m_Block->Allocator.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p) // Lock cannot be noexcept
		{
			std::lock_guard<std::mutex> lock(m_Block->Lock);

			m_Block->Allocator.destroy(p);
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const
			noexcept(noexcept(m_Block->Allocator.max_size()))
		{
			return m_Block->Allocator.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
			noexcept(noexcept(m_Block->Allocator.owns(p)))
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
			noexcept(noexcept(detail::aligned_delete(m_Block)))
		{
			if (!m_Block) return;

			if (m_Block->UseCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
				detail::aligned_delete(m_Block);
		}

	private:
		block* m_Block;
	};
}