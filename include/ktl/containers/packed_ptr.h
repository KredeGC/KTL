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
	template<typename PtrT, typename IntT, size_t Bits, IntT Min, IntT Max, size_t Alignment>
	class packed_ptr
	{
	public:
		static constexpr uintmax_t FREE_BITS = detail::log2(Alignment);

	private:
		static_assert(std::is_pointer_v<PtrT>, "Type must be a pointer");
		static_assert(Bits <= FREE_BITS, "The number of bits in use cannot surpass the number of free bits");
		static_assert(std::is_integral_v<IntT>, "The packed type must be an integer");
		//static_assert(std::is_unsigned_v<IntT>, "Packed integer must be unsigned");

		static constexpr uintptr_t INT_MASK = (1ULL << Bits) - 1ULL;
		static constexpr uintptr_t PTR_MASK = ~((1ULL << FREE_BITS) - 1ULL);
		static constexpr uintptr_t SIGNED_INT_MIN = 1ULL << (Bits - 1ULL);

	public:
		packed_ptr() noexcept :
			m_Value(reinterpret_cast<uintptr_t>(nullptr)) {}

		packed_ptr(PtrT p, IntT value) noexcept :
			m_Value(from_ptr(p) | from_int(value))
		{
			// Pointer must be correctly aligned
			KTL_ASSERT((reinterpret_cast<size_t>(p) & (Alignment - 1)) == 0);
		}

		PtrT get_ptr() const noexcept { return to_ptr(m_Value); }

		IntT get_int() const noexcept { return to_int(m_Value); }

		void set_ptr(PtrT p) noexcept
		{
			// Pointer must be correctly aligned
			KTL_ASSERT((reinterpret_cast<size_t>(p) & (Alignment - 1)) == 0);

			m_Value = from_ptr(p) | (m_Value & INT_MASK);
		}

		void set_int(IntT value) noexcept
		{
			KTL_ASSERT(value >= Min && value <= Max);

			m_Value = (m_Value & PTR_MASK) | from_int(value);
		}

	private:
		PtrT to_ptr(uintptr_t value) const noexcept
		{
			return reinterpret_cast<PtrT>(value & PTR_MASK);
		}

		uintptr_t from_ptr(PtrT value) const noexcept
		{
			return reinterpret_cast<uintptr_t>(value) & PTR_MASK;
		}

		IntT to_int(uintptr_t value) const noexcept
		{
			return static_cast<IntT>(value & INT_MASK) + Min;
		}

		uintptr_t from_int(IntT value) const noexcept
		{
			return static_cast<uintptr_t>(value - Min) & INT_MASK;
		}

	private:
		uintptr_t m_Value;
	};
}