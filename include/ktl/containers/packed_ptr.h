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
	 * @tparam IntT The integer type to use
	*/
	template<typename PtrT, typename IntT, IntT Min, IntT Max, size_t Alignment>
	class packed_ptr
	{
	public:
		using ptr_type = std::conditional_t<std::is_function_v<PtrT>, PtrT*, PtrT>;
		using int_type = detail::underlying_type_t<IntT>;

		static constexpr uintmax_t UsedBits = detail::bits_in_range(static_cast<int64_t>(Min), static_cast<int64_t>(Max));
		static constexpr uintmax_t FreeBits = detail::log2(Alignment);

	private:
		static_assert(!std::is_const_v<PtrT>, "Pointer type cannot be const");
		static_assert(!std::is_const_v<IntT>, "Integer type cannot be const");
		static_assert(std::is_pointer_v<PtrT> || std::is_function_v<PtrT>, "Type must be a pointer");
		static_assert(UsedBits <= FreeBits, "The number of bits in use cannot surpass the number of free bits");
		static_assert(std::is_integral_v<IntT> || std::is_enum_v<IntT>, "The packed type must be an integer, bool or enum");

		static constexpr uintptr_t INT_MASK = (1ULL << UsedBits) - 1ULL;
		static constexpr uintptr_t PTR_MASK = ~((1ULL << FreeBits) - 1ULL);
		static constexpr uintptr_t SIGNED_INT_MIN = 1ULL << (UsedBits - 1ULL);

	public:
		packed_ptr() noexcept :
			m_Value(reinterpret_cast<uintptr_t>(nullptr)) {}

		packed_ptr(PtrT p, IntT value) noexcept :
			m_Value(from_ptr(p) | from_int(value))
		{
			// Pointer must be correctly aligned
			KTL_ASSERT((reinterpret_cast<size_t>(p) & (Alignment - 1)) == 0);

			// Integer must be between min and max
			KTL_ASSERT(value >= Min && value <= Max);
		}

		packed_ptr(PtrT p) noexcept :
			m_Value(from_ptr(p))
		{
			// Pointer must be correctly aligned
			KTL_ASSERT((reinterpret_cast<size_t>(p) & (Alignment - 1)) == 0);
		}

		packed_ptr(IntT value) noexcept :
			m_Value(from_int(value))
		{
			// Integer must be between min and max
			KTL_ASSERT(value >= Min && value <= Max);
		}

#ifndef KTL_EXPLICIT_POINTER
		packed_ptr& operator=(PtrT p) noexcept
		{
			set_ptr(p);
			return *this;
		}

		packed_ptr& operator=(IntT value) noexcept
		{
			set_int(value);
			return *this;
		}

		friend bool operator==(packed_ptr pack, PtrT p) noexcept { return pack.get_ptr() == p; }

		friend bool operator==(packed_ptr pack, IntT n) noexcept { return pack.get_int() == n; }

		friend bool operator==(PtrT p, packed_ptr pack) noexcept { return pack.get_ptr() == p; }

		friend bool operator==(IntT n, packed_ptr pack) noexcept { return pack.get_int() == n; }

		template<typename Y = IntT, typename = std::enable_if_t<!std::is_same_v<Y, bool>, Y>>
		operator ptr_type() const noexcept { return get_ptr(); }

		template<typename Y = IntT, typename = std::enable_if_t<!std::is_same_v<Y, bool>, Y>>
		operator IntT() const noexcept { return get_int(); }

		explicit operator bool() const noexcept { return get_ptr(); }
#endif // KTL_EXPLICIT_POINTER

		ptr_type operator->() const noexcept { return get_ptr(); }

		decltype(*std::declval<PtrT>()) operator*() const noexcept { return *get_ptr(); }

		template<typename... Ts>
		decltype(auto) operator()(Ts&&... args) const noexcept { return get_ptr()(std::forward<Ts>(args) ...); }

		ptr_type get_ptr() const noexcept { return to_ptr(m_Value); }

		IntT get_int() const noexcept { return to_int(m_Value); }

		void set_ptr(PtrT p) noexcept
		{
			// Pointer must be correctly aligned
			KTL_ASSERT((reinterpret_cast<size_t>(p) & (Alignment - 1)) == 0);

			m_Value = from_ptr(p) | (m_Value & INT_MASK);
		}

		void set_int(IntT value) noexcept
		{
			// Integer must be between min and max
			KTL_ASSERT(value >= Min && value <= Max);

			m_Value = (m_Value & PTR_MASK) | from_int(value);
		}

	private:
		ptr_type to_ptr(uintptr_t value) const noexcept
		{
			return reinterpret_cast<ptr_type>(value & PTR_MASK);
		}

		uintptr_t from_ptr(PtrT value) const noexcept
		{
			return reinterpret_cast<uintptr_t>(value) & PTR_MASK;
		}

		IntT to_int(uintptr_t value) const noexcept
		{
			return static_cast<IntT>(static_cast<int_type>(value & INT_MASK) + static_cast<int_type>(Min));
		}

		uintptr_t from_int(IntT value) const noexcept
		{
			return static_cast<uintptr_t>(static_cast<int_type>(value) - static_cast<int_type>(Min)) & INT_MASK;
		}

	private:
		uintptr_t m_Value;
	};
}