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
		static_assert(has_value_type<P>(), "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(has_value_type<F>(), "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(!has_no_owns<P>(), "The primary allocator is required to have an 'own(void*)' method");

	public:
		using size_type = typename P::size_type;

		composite_allocator(const P& primary = P(), const F& fallback = F()) noexcept :
			m_Primary(primary),
			m_Fallback(fallback) {}

		composite_allocator(const composite_allocator& other) noexcept :
			m_Primary(other.m_Primary),
			m_Fallback(other.m_Fallback) {}

		void* allocate(size_t n)
		{
			void* ptr = m_Primary.allocate(n);
			if (!ptr)
				return m_Fallback.allocate(n);
			return ptr;
		}

		void deallocate(void* p, size_t n)
		{
			if constexpr (!has_no_owns<P>::value)
			{
				if (m_Primary.owns(p))
				{
					m_Primary.deallocate(p, n);
					return;
				}
			}

			m_Fallback.deallocate(p, n);
		}

		template<typename T, typename... Args>
		void construct(T* p, Args&&... args)
		{
			if constexpr (!has_no_owns<P>::value && !has_no_construct<void, P, T*, Args...>::value)
				m_Primary.construct(p, std::forward<Args>(args)...);
			else if constexpr (!has_no_owns<F>::value && !has_no_construct<void, F, T*, Args...>::value)
				m_Fallback.construct(p, std::forward<Args>(args)...);
			else
				::new (p) T(std::forward<Args>(args)...);
		}

		template<typename T>
		void destroy(T* p)
		{
			if constexpr (!has_no_owns<P>::value && !has_no_destroy<void, P, T*>::value)
				m_Primary.destroy(p);
			else if constexpr (!has_no_owns<F>::value && !has_no_destroy<void, F, T*>::value)
				m_Fallback.destroy(p);
			else
				p->~T();
		}

		size_type max_size() const noexcept
		{
			if constexpr (!has_no_max_size<P>::value && !has_no_max_size<F>::value)
				return (std::max)(m_Primary.max_size(), m_Fallback.max_size());
			else
				return std::numeric_limits<size_type>::max();
		}

		bool owns(void* p)
		{
			if constexpr (!has_no_owns<P>::value)
			{
				if (m_Primary.owns(p))
					return true;
			}
			else if constexpr (!has_no_owns<F>::value)
			{
				if (m_Fallback.owns(p))
					return true;
			}
			return false;
		}

	private:
		P m_Primary;
		F m_Fallback;
	};

	template<typename P, typename F, typename U, typename V>
	bool operator==(const composite_allocator<P, F>&, const composite_allocator<U, V>&) noexcept
	{
		return true;
	}

	template<typename P, typename F, typename U, typename V>
	bool operator!=(const composite_allocator<P, F>&, const composite_allocator<U, V>&) noexcept
	{
		return false;
	}

	template<typename T, typename P, typename F>
	using type_composite_allocator = type_allocator<T, composite_allocator<P, F>>;
}