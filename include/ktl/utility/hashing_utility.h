#pragma once

#include <cstddef>
#include <cstdint>

namespace ktl
{
	inline constexpr uint8_t FLAG_OCCUPIED = 0x01;
	inline constexpr uint8_t FLAG_DEAD = 0x02;

	inline constexpr bool flag_occupied(size_t flags)
	{
		return (flags & FLAG_OCCUPIED) != 0;
	}

	inline constexpr bool flag_occupied_alive(size_t flags)
	{
		return (flags & (FLAG_OCCUPIED | FLAG_DEAD)) == FLAG_OCCUPIED;
	}

	inline constexpr bool flag_dead(size_t flags)
	{
		return (flags & FLAG_DEAD) != 0;
	}

	inline constexpr size_t size_pow2(size_t n) noexcept
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

	inline constexpr size_t hash_collision_offset(size_t key, size_t counter, size_t mask) noexcept
	{
		// Linear probing for best cache locality
		return (key + counter) & mask;
	}
}