#pragma once

#include <cstddef>
#include <type_traits>

namespace ktl
{
	namespace detail
	{
		template<typename T>
		constexpr T min_range(size_t bits)
		{
			if constexpr (std::is_unsigned_v<T>)
				return static_cast<T>(0ULL);
			else
				return -static_cast<T>(1ULL << (bits - 1ULL));
		}

		template<typename T>
		constexpr T max_range(size_t bits)
		{
			return min_range<T>(bits) + static_cast<T>((1ULL << bits) - 1ULL);
		}
	}

	template<
		typename PtrT,
		typename IntT,
		size_t Bits = detail::log2(alignof(std::remove_pointer_t<PtrT>)),
		IntT Min = detail::min_range<IntT>(Bits),
		IntT Max = detail::max_range<IntT>(Bits),
		size_t Alignment = alignof(std::remove_pointer_t<PtrT>)>
	class packed_ptr;
}