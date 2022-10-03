#pragma once

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename P, typename F>
	class composite_allocator : public P, private F
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

		composite_allocator() noexcept : P(), F() {}

		template<typename U, typename V>
		composite_allocator(const composite_allocator<U, V>& other) noexcept : P(), F() {}

		value_type* allocate(size_t n)
		{
			value_type* ptr = primary_traits::allocate(*this, n);
			if (!ptr)
				return fallback_traits::allocate(*this, n);
			return ptr;
		}

		void deallocate(value_type* p, size_t n)
		{
			if (P::owns(p))
				primary_traits::deallocate(*this, p, n);
			else
				fallback_traits::deallocate(*this, p, n);
		}

		size_type max_size() const noexcept
		{
			return (std::max)(primary_traits::max_size(*this), fallback_traits::max_size(*this));
		}

		bool owns(value_type* p)
		{
			return P::owns(p) || F::owns(p);
		}
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