#pragma once

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename P, typename F>
	class composite_allocator : public P, private F
	{
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
			value_type* ptr = P::allocate(n);
			if (!ptr)
				return F::allocate(n);
			return ptr;
		}

		void deallocate(value_type* p, size_t n)
		{
			if (P::owns(p))
				P::deallocate(p, n);
			else
				F::deallocate(p, n);
		}

		size_type max_size() const noexcept
		{
			return (std::max)(P::max_size(), F::max_size());
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