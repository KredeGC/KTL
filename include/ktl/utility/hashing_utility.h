#pragma once

#include <cstddef>

namespace ktl
{


	static constexpr size_t size_pow2(size_t n) noexcept
	{
		// Find the second power of 2 above n
		n--;
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		n |= n >> 32;
		n += n + 2;

		return n;
	}

	static constexpr size_t hash_collision_offset(size_t key, size_t counter, size_t mask) noexcept
	{
		// Linear probing for best cache locality
		return (key + counter) & mask;
	}
}