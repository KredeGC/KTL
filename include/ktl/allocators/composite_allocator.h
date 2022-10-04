#pragma once

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename P, typename F>
	class composite_allocator
	{
	private:
		using primary_traits = std::allocator_traits<P>;
		using fallback_traits = std::allocator_traits<F>;

	public:
		using value_type = typename P::value_type;
		using size_type = typename P::size_type;
		using is_always_equal = std::true_type;

		template<typename U, typename V>
		struct rebind
		{
			using other = composite_allocator<U, V>;
		};

		composite_allocator(const P& primary = P(), const F& fallback = F()) noexcept :
			m_Primary(primary),
			m_Fallback(fallback) {}

		composite_allocator(const composite_allocator& other) noexcept :
			m_Primary(other.m_Primary),
			m_Fallback(other.m_Fallback) {}

		template<typename U, typename V>
		composite_allocator(const composite_allocator<U, V>& other) noexcept :
			m_Primary(other.m_Primary),
			m_Fallback(other.m_Fallback) {}

		value_type* allocate(size_t n)
		{
			value_type* ptr = primary_traits::allocate(m_Primary, n);
			if (!ptr)
				return fallback_traits::allocate(m_Fallback, n);
			return ptr;
		}

		void deallocate(value_type* p, size_t n)
		{
			if (m_Primary.owns(p))
				primary_traits::deallocate(m_Primary, p, n);
			else
				fallback_traits::deallocate(m_Fallback, p, n);
		}

		size_type max_size() const noexcept
		{
			return (std::max)(primary_traits::max_size(m_Primary), fallback_traits::max_size(m_Fallback));
		}

		bool owns(value_type* p)
		{
			return m_Primary.owns(p) || m_Fallback.owns(p);
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
}