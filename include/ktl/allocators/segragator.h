#pragma once

#include "../utility/meta.h"
#include "segragator_fwd.h"
#include "type_allocator.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	/**
	 * @brief An allocator which delegates allocations between 2 different allocators based on a size threshold.
	 * @note The primary allocator must have an owns(*ptr) method if it also has a construct() method.
	 * @tparam P The primary allocator, which is used when size is less than or equal to Threshold
	 * @tparam F The fallback allocator, which is used when size is bigger than Threshold
	*/
	template<size_t Threshold, typename P, typename F>
	class segragator
	{
	private:
		static_assert(detail::has_no_value_type<P>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(detail::has_no_value_type<F>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(!detail::has_construct<P>::value || detail::has_owns<P>::value, "The primary allocator is required to have an 'owns(void*)' method, if it has a construct(void*, Args...) method");

	public:
		typedef typename detail::get_size_type<P>::type size_type;

		segragator() noexcept :
			m_Primary(),
			m_Fallback() {}

		template<typename Primary, typename = std::enable_if_t<std::is_convertible_v<Primary, P>>>
		segragator(Primary&& primary) noexcept :
			m_Primary(std::forward<Primary>(primary)),
			m_Fallback() {}

		template<typename Primary, typename Fallback,
			typename = std::enable_if_t<std::is_convertible_v<Primary, P> && std::is_convertible_v<Fallback, F>>>
		segragator(Primary&& primary, Fallback&& fallback) noexcept :
			m_Primary(std::forward<Primary>(primary)),
			m_Fallback(std::forward<Fallback>(fallback)) {}

		segragator(const segragator&) noexcept = default;

		segragator(segragator&&) noexcept = default;

		segragator& operator=(const segragator&) noexcept = default;

		segragator& operator=(segragator&&) noexcept = default;

		bool operator==(const segragator& rhs) const noexcept
		{
			return m_Primary == rhs.m_Primary && m_Fallback == rhs.m_Fallback;
		}

		bool operator!=(const segragator& rhs) const noexcept
		{
			return m_Primary != rhs.m_Primary || m_Fallback != rhs.m_Fallback;
		}

#pragma region Allocation
		void* allocate(size_t n)
		{
			if (n <= Threshold)
				return m_Primary.allocate(n);
			else
				return m_Fallback.allocate(n);
		}

		void deallocate(void* p, size_t n)
		{
			if (n <= Threshold)
				return m_Primary.deallocate(p, n);
			else
				return m_Fallback.deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct<void, P, T*, Args...>::value || detail::has_construct<void, F, T*, Args...>::value, void>::type
		construct(T* p, Args&&... args)
		{
			bool owned = m_Primary.owns(p);

			if constexpr (detail::has_construct<void, P, T*, Args...>::value)
			{
				if (owned)
				{
					m_Primary.construct(p, std::forward<Args>(args)...);
					return;
				}
			}

			if constexpr (detail::has_construct<void, F, T*, Args...>::value)
			{
				if (!owned)
				{
					m_Fallback.construct(p, std::forward<Args>(args)...);
					return;
				}
			}

			::new(p) T(std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy<P, T*>::value || detail::has_destroy<F, T*>::value, void>::type
		destroy(T* p)
		{
			bool owned = m_Primary.owns(p);

			if constexpr (detail::has_destroy<P, T*>::value)
			{
				if (owned)
				{
					m_Primary.destroy(p);
					return;
				}
			}

			if constexpr (detail::has_destroy<F, T*>::value)
			{
				if (!owned)
				{
					m_Fallback.destroy(p);
					return;
				}
			}

			p->~T();
		}
#pragma endregion

#pragma region Utility
		template<typename Primary = P, typename Fallback = F>
		typename std::enable_if<detail::has_max_size<Primary>::value && detail::has_max_size<Fallback>::value, size_type>::type
		max_size() const noexcept
		{
			return (std::max)(m_Primary.max_size(), m_Fallback.max_size());
		}

		template<typename Primary = P, typename Fallback = F>
		typename std::enable_if<detail::has_owns<Primary>::value && detail::has_owns<Fallback>::value, bool>::type
		owns(void* p) const
		{
			if (m_Primary.owns(p))
				return true;

			return m_Fallback.owns(p);
		}
#pragma endregion

	private:
		P m_Primary;
		F m_Fallback;
	};
}