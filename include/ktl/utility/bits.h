#pragma once

#include <cstdint>

namespace ktl::detail
{
	constexpr inline uintmax_t log2(uintmax_t n) noexcept
	{
		uintmax_t r = 0;

		if (n >> 32) { r += 32U; n >>= 32U; }
		if (n >> 16) { r += 16U; n >>= 16U; }
		if (n >> 8) { r += 8U; n >>= 8U; }
		if (n >> 4) { r += 4U; n >>= 4U; }
		if (n >> 2) { r += 2U; n >>= 2U; }
		if (n >> 1) { r += 1U; n >>= 1U; }

		return r;
	}
}