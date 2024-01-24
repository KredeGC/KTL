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

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

		template<typename A = Alloc>
		threaded()
			noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
			m_Alloc(),
			m_Lock() {}

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			std::is_constructible_v<Alloc, Args...>>>
		explicit threaded(Args&&... args)
			noexcept(std::is_nothrow_constructible_v<Alloc, Args...>) :
			m_Alloc(std::forward<Args>(args) ...),
			m_Lock() {}

		threaded(const threaded&) = delete;
		threaded(threaded&&) = delete;

		threaded& operator=(const threaded&) = delete;
		threaded& operator=(threaded&&) = delete;

		bool operator==(const threaded& rhs) const
			noexcept(detail::has_nothrow_equal_v<Alloc>)
		{
			return m_Alloc == rhs.m_Alloc;
		}

		bool operator!=(const threaded& rhs) const
			noexcept(detail::has_nothrow_not_equal_v<Alloc>)
		{
			return m_Alloc != rhs.m_Alloc;
		}

#pragma region Allocation
		void* allocate(size_t n) // Lock cannot be noexcept
		{
			std::lock_guard<std::mutex> lock(m_Lock);

			return m_Alloc.allocate(n);
		}

		void deallocate(void* p, size_t n) // Lock cannot be noexcept
		{
			std::lock_guard<std::mutex> lock(m_Lock);

			m_Alloc.deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args) // Lock cannot be noexcept
		{
			std::lock_guard<std::mutex> lock(m_Lock);

			m_Alloc.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p) // Lock cannot be noexcept
		{
			std::lock_guard<std::mutex> lock(m_Lock);

			m_Alloc.destroy(p);
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const
			noexcept(detail::has_nothrow_max_size_v<A>)
		{
			return m_Alloc.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
			noexcept(detail::has_nothrow_owns_v<A>)
		{
			return m_Alloc.owns(p);
		}
#pragma endregion

		Alloc& get_allocator() noexcept
		{
			return m_Alloc;
		}

		const Alloc& get_allocator() const noexcept
		{
			return m_Alloc;
		}

	private:
		KTL_EMPTY_BASE Alloc m_Alloc;
		std::mutex m_Lock;
	};
}