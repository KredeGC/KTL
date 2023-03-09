#pragma once

#include "../utility/meta.h"
#include "threaded_fwd.h"

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

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			detail::can_construct_v<Alloc, Args...>>>
		threaded(Args&&... alloc) noexcept :
			m_Alloc(std::forward<Args>(alloc)...) {}

		threaded(const threaded& other) noexcept = default;

		threaded(threaded&& other) noexcept = default;

		threaded& operator=(const threaded& rhs) noexcept = default;

		threaded& operator=(threaded&& rhs) noexcept = default;

		bool operator==(const threaded& rhs) const noexcept
		{
			return m_Alloc == rhs.m_Alloc;
		}

		bool operator!=(const threaded& rhs) const noexcept
		{
			return m_Alloc != rhs.m_Alloc;
		}

#pragma region Allocation
		void* allocate(size_t n)
		{
			std::lock_guard<std::mutex> lock(m_Lock);

			return m_Alloc.allocate(n);
		}

		void deallocate(void* p, size_t n)
		{
			std::lock_guard<std::mutex> lock(m_Lock);

			m_Alloc.deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args)
		{
			std::lock_guard<std::mutex> lock(m_Lock);

			m_Alloc.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p)
		{
			std::lock_guard<std::mutex> lock(m_Lock);

			m_Alloc.destroy(p);
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const noexcept
		{
			return m_Alloc.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
		{
			return m_Alloc.owns(p);
		}
#pragma endregion

		Alloc& get_allocator()
		{
			return m_Alloc;
		}

		const Alloc& get_allocator() const
		{
			return m_Alloc;
		}

	private:
		Alloc m_Alloc;
		std::mutex m_Lock;
	};
}