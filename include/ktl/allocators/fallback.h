#pragma once

#include "../utility/empty_base.h"
#include "../utility/meta.h"
#include "fallback_fwd.h"
#include "type_allocator.h"

#include <limits>
#include <memory>
#include <type_traits>

namespace ktl
{
	/**
	 * @brief An allocator which delegates allocations between 2 different allocators.
	 * It first attempts to allocate with the primary allocator, but upon failure will use the fallback allocator.
	 * @note The primary allocator must have an owns(*ptr) method.
	 * @tparam P The primary allocator
	 * @tparam F The fallback allocator, which is used when the primary allocator fails
	*/
	template<typename P, typename F>
	class fallback
	{
	private:
		static_assert(detail::has_no_value_type_v<P>, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(detail::has_no_value_type_v<F>, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(detail::has_owns_v<P>, "The primary allocator is required to have an 'owns(void*)' method");

	public:
		typedef typename detail::get_size_type_t<P> size_type;

		fallback()
			noexcept(std::is_nothrow_default_constructible_v<P> && std::is_nothrow_default_constructible_v<F>) :
			m_Primary(),
			m_Fallback() {}

		/**
		 * @brief Constructor for forwarding a single argument to the primary allocator
		*/
		template<typename Primary,
			typename = std::enable_if_t<detail::can_construct_v<P, Primary>>>
		explicit fallback(Primary&& primary)
			noexcept(std::is_nothrow_constructible_v<P, Primary> && std::is_nothrow_default_constructible_v<F>) :
			m_Primary(std::forward<Primary>(primary)),
			m_Fallback() {}

		/**
		 * @brief Constructor for forwarding a single argument to the primary and fallback allocators
		*/
		template<typename Primary, typename Fallback,
			typename = std::enable_if_t<
			detail::can_construct_v<P, Primary> &&
			detail::can_construct_v<F, Fallback>>>
		explicit fallback(Primary&& primary, Fallback&& fallback)
			noexcept(std::is_nothrow_constructible_v<P, Primary> && std::is_nothrow_constructible_v<F, Fallback>) :
			m_Primary(std::forward<Primary>(primary)),
			m_Fallback(std::forward<Fallback>(fallback)) {}

		/**
		 * @brief Constructor for forwarding a tuple of arguments to the primary allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			detail::can_construct_v<P, Args...>>>
		explicit fallback(std::tuple<Args...>&& primary)
			noexcept(std::is_nothrow_constructible_v<P, Args...> && std::is_nothrow_default_constructible_v<F>) :
			m_Primary(std::make_from_tuple<P>(std::forward<std::tuple<Args...>>(primary))),
			m_Fallback() {}

		/**
		 * @brief Constructor for forwarding a tuple of arguments to the primary and fallback allocators
		*/
		template<typename... ArgsP, typename... ArgsF,
			typename = std::enable_if_t<
			detail::can_construct_v<P, ArgsP...>&&
			detail::can_construct_v<F, ArgsF...>>>
		explicit fallback(std::tuple<ArgsP...>&& primary, std::tuple<ArgsF...>&& fallback)
			noexcept(std::is_nothrow_constructible_v<P, ArgsP...> && std::is_nothrow_constructible_v<F, ArgsF...>) :
			m_Primary(std::make_from_tuple<P>(std::forward<std::tuple<ArgsP...>>(primary))),
			m_Fallback(std::make_from_tuple<F>(std::forward<std::tuple<ArgsF...>>(fallback))) {}

		fallback(const fallback&) = default;

		fallback(fallback&&) = default;

		fallback& operator=(const fallback&) = default;

		fallback& operator=(fallback&&) = default;

		bool operator==(const fallback& rhs) const
			noexcept(noexcept(m_Primary == rhs.m_Primary) && noexcept(m_Fallback == rhs.m_Fallback))
		{
			return m_Primary == rhs.m_Primary && m_Fallback == rhs.m_Fallback;
		}

		bool operator!=(const fallback& rhs) const
			noexcept(noexcept(m_Primary != rhs.m_Primary) && noexcept(m_Fallback != rhs.m_Fallback))
		{
			return m_Primary != rhs.m_Primary || m_Fallback != rhs.m_Fallback;
		}

#pragma region Allocation
		void* allocate(size_t n)
			noexcept(noexcept(m_Primary.allocate(n)) && noexcept(m_Fallback.allocate(n)))
		{
			void* ptr = m_Primary.allocate(n);
			if (!ptr)
				return m_Fallback.allocate(n);
			return ptr;
		}

		void deallocate(void* p, size_t n)
			noexcept(noexcept(m_Primary.deallocate(p, n)) && noexcept(m_Fallback.deallocate(p, n)))
		{
			if (m_Primary.owns(p))
			{
				m_Primary.deallocate(p, n);
				return;
			}

			m_Fallback.deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<P, T*, Args...> || detail::has_construct_v<F, T*, Args...>, void>::type
		construct(T* p, Args&&... args) noexcept(
			(!detail::has_construct_v<P, T*, Args...> || detail::has_noexcept_construct_v<P, T*, Args...>) &&
			(!detail::has_construct_v<F, T*, Args...> || detail::has_noexcept_construct_v<F, T*, Args...>) &&
			std::is_nothrow_constructible_v<T, Args...>)
		{
			bool owned = m_Primary.owns(p);

			if constexpr (detail::has_construct_v<P, T*, Args...>)
			{
				if (owned)
				{
					m_Primary.construct(p, std::forward<Args>(args)...);
					return;
				}
			}

			if constexpr (detail::has_construct_v<F, T*, Args...>)
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
		typename std::enable_if<detail::has_destroy_v<P, T*> || detail::has_destroy_v<F, T*>, void>::type
		destroy(T* p) noexcept(
			(!detail::has_destroy_v<P, T*> || detail::has_noexcept_destroy_v<P, T*>) &&
			(!detail::has_destroy_v<F, T*> || detail::has_noexcept_destroy_v<F, T*>) &&
			std::is_nothrow_destructible_v<T>)
		{
			bool owned = m_Primary.owns(p);

			if constexpr (detail::has_destroy_v<P, T*>)
			{
				if (owned)
				{
					m_Primary.destroy(p);
					return;
				}
			}

			if constexpr (detail::has_destroy_v<F, T*>)
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
		typename std::enable_if<detail::has_max_size_v<Primary> && detail::has_max_size_v<Fallback>, size_type>::type
		max_size() const
			noexcept(noexcept(m_Primary.max_size()) && noexcept(m_Fallback.max_size()))
		{
			return (std::max)(m_Primary.max_size(), m_Fallback.max_size());
		}

		template<typename Primary = P, typename Fallback = F>
		typename std::enable_if<detail::has_owns_v<Primary> && detail::has_owns_v<Fallback>, bool>::type
		owns(void* p) const
			noexcept(noexcept(m_Primary.owns(p)) && noexcept(m_Fallback.owns(p)))
		{
			if (m_Primary.owns(p))
				return true;
			
			return m_Fallback.owns(p);
		}
#pragma endregion

	private:
		KTL_EMPTY_BASE P m_Primary;
		KTL_EMPTY_BASE F m_Fallback;
	};
}