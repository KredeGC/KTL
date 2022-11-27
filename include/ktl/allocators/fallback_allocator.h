#pragma once

#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <limits>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename P, typename F>
	class fallback_allocator
	{
	private:
		static_assert(has_no_value_type<P>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(has_no_value_type<F>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(has_owns<P>::value, "The primary allocator is required to have an 'owns(void*)' method");

	public:
		typedef typename get_size_type<P>::type size_type;

		fallback_allocator(const P& primary = P(), const F& fallback = F()) noexcept :
			m_Primary(primary),
			m_Fallback(fallback) {}

		fallback_allocator(const fallback_allocator& other) noexcept = default;

		fallback_allocator(fallback_allocator&& other) noexcept = default;

		fallback_allocator& operator=(const fallback_allocator& rhs) noexcept = default;

		fallback_allocator& operator=(fallback_allocator&& rhs) noexcept = default;

		bool operator==(const fallback_allocator& rhs) const noexcept
		{
			return m_Primary == rhs.m_Primary && m_Fallback == rhs.m_Fallback;
		}

		bool operator!=(const fallback_allocator& rhs) const noexcept
		{
			return m_Primary != rhs.m_Primary || m_Fallback != rhs.m_Fallback;
		}

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
			bool owned = m_Primary.owns(p);

			if constexpr (has_construct<void, P, T*, Args...>::value)
			{
				if (owned)
				{
					m_Primary.construct(p, std::forward<Args>(args)...);
					return;
				}
			}

			if constexpr (has_construct<void, F, T*, Args...>::value)
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
		typename std::enable_if<has_destroy<P, T*>::value || has_destroy<F, T*>::value, void>::type
		destroy(T* p)
		{
			bool owned = m_Primary.owns(p);

			if constexpr (has_destroy<P, T*>::value)
			{
				if (owned)
				{
					m_Primary.destroy(p);
					return;
				}
			}

			if constexpr (has_destroy<F, T*>::value)
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
		typename std::enable_if<has_max_size<Primary>::value && has_max_size<Fallback>::value, size_type>::type
		max_size() const noexcept
		{
			return (std::max)(m_Primary.max_size(), m_Fallback.max_size());
		}

		template<typename Primary = P, typename Fallback = F>
		typename std::enable_if<has_owns<Primary>::value && has_owns<Fallback>::value, bool>::type
		owns(void* p)
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

	template<typename T, typename P, typename F>
	using type_fallback_allocator = type_allocator<T, fallback_allocator<P, F>>;
}