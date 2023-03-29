#pragma once

#include "../utility/assert.h"
#include "../utility/bits.h"
#include "packed_ptr_fwd.h"

#include <type_traits>

namespace ktl
{
	/**
	 * @brief A packed pointer-like type which takes advantage of any bits that don't get used due to alignment
	 * @tparam PtrT The pointer type to use
	*/
	template<typename PtrT, size_t Bits, size_t Alignment>
	class packed_ptr
	{
	public:
		static constexpr uintmax_t FREE_BITS = detail::log2(Alignment);

	private:
		static_assert(Bits <= FREE_BITS, "The number of bits in use cannot surpass the number of free bits");

		static constexpr uintptr_t INT_MASK = ((1ULL << Bits) - 1);
		static constexpr uintptr_t PTR_MASK = ~((1ULL << FREE_BITS) - 1);

	public:
		packed_ptr() noexcept :
			m_Value(reinterpret_cast<uintptr_t>(nullptr)) {}

		template<typename Int>
		explicit packed_ptr(PtrT p, Int value) noexcept :
			m_Value((reinterpret_cast<uintptr_t>(p) & PTR_MASK) | (static_cast<uintptr_t>(value) & INT_MASK))
		{
			// Pointer must be correctly aligned
			KTL_ASSERT((reinterpret_cast<size_t>(p) & (Alignment - 1)) == 0);
		}

		operator bool() const noexcept { return m_Value; }

		PtrT get_ptr() const noexcept { return reinterpret_cast<PtrT>(m_Value & PTR_MASK); }

		template<typename Int>
		Int get_int() const noexcept
		{
			static_assert(std::is_unsigned_v<Int>, "Packed integer must be unsigned");

			return static_cast<Int>(m_Value & INT_MASK);
		}

		void set_ptr(PtrT p) noexcept
		{
			// Pointer must be correctly aligned
			KTL_ASSERT((reinterpret_cast<size_t>(p) & (Alignment - 1)) == 0);

			m_Value = (reinterpret_cast<uintptr_t>(p) & PTR_MASK) | (m_Value & INT_MASK);
		}

		template<typename Int>
		void set_int(Int value) noexcept
		{
			static_assert(std::is_unsigned_v<Int>, "Packed integer must be unsigned");

			m_Value = (m_Value & PTR_MASK) | (static_cast<uintptr_t>(value) & INT_MASK);
		}

	private:
		uintptr_t m_Value;
	};
}