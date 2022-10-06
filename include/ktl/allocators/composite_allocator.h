#pragma once

#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <limits>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename P, typename F>
	class composite_allocator
	{
	private:
		static_assert(has_value_type<P>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(has_value_type<F>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(has_owns<P>::value, "The primary allocator is required to have an 'owns(void*)' method");

	public:
		typedef typename get_size_type<P>::type size_type;

		composite_allocator(const P& primary = P(), const F& fallback = F()) noexcept :
			m_Primary(primary),
			m_Fallback(fallback) {}

		composite_allocator(const composite_allocator& other) noexcept :
			m_Primary(other.m_Primary),
			m_Fallback(other.m_Fallback) {}

#pragma region Allocation
		void* allocate(size_t n)
		{
			void* ptr = m_Primary.allocate(n);
			if (!ptr)
				return m_Fallback.allocate(n);
			return ptr;
		}

		void deallocate(void* p, size_t n)
		{
			if constexpr (has_owns<P>::value)
			{
				if (m_Primary.owns(p))
				{
					m_Primary.deallocate(p, n);
					return;
				}
			}

			m_Fallback.deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<has_construct<void, P, T*, Args...>::value || has_construct<void, F, T*, Args...>::value, void>::type
		construct(T* p, Args&&... args)
		{
			if constexpr (has_construct<void, P, T*, Args...>::value)
			{
				if (m_Primary.owns(p))
				{
					m_Primary.construct(p, std::forward<Args>(args)...);
					return;
				}
			}

			if constexpr (has_construct<void, F, T*, Args...>::value)
			{
				m_Fallback.construct(p, std::forward<Args>(args)...);
				return;
			}

			::new(p) T(std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<has_destroy<P, T*>::value || has_destroy<F, T*>::value, void>::type
		destroy(T* p)
		{
			if constexpr (has_destroy<P, T*>::value)
			{
				if (m_Primary.owns(p))
				{
					m_Primary.destroy(p);
					return;
				}
			}

			if constexpr (has_destroy<F, T*>::value)
			{
				m_Fallback.destroy(p);
				return;
			}

			p->~T();
		}
#pragma endregion

#pragma region Utility
		template<typename Pr = P, typename Fr = F>
		typename std::enable_if<has_max_size<Pr>::value && has_max_size<Fr>::value, size_type>::type
		max_size() const noexcept
		{
			return (std::max)(m_Primary.max_size(), m_Fallback.max_size());
		}

		bool owns(void* p)
		{
			if constexpr (has_owns<P>::value)
			{
				if (m_Primary.owns(p))
					return true;
			}
			else if constexpr (has_owns<F>::value)
			{
				if (m_Fallback.owns(p))
					return true;
			}
			return false;
		}
#pragma endregion

	private:
		P m_Primary;
		F m_Fallback;
	};

	template<typename P, typename F, typename U, typename V>
	bool operator==(const composite_allocator<P, F>& lhs, const composite_allocator<U, V>& rhs) noexcept
	{
		return lhs.m_Primary == rhs.m_Primary && lhs.m_Fallback == rhs.m_Fallback;
	}

	template<typename P, typename F, typename U, typename V>
	bool operator!=(const composite_allocator<P, F>& lhs, const composite_allocator<U, V>& rhs) noexcept
	{
		return lhs.m_Primary != rhs.m_Primary || lhs.m_Fallback != rhs.m_Fallback;
	}

	template<typename T, typename P, typename F>
	using type_composite_allocator = type_allocator<T, composite_allocator<P, F>>;
}