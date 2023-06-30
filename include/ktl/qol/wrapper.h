#pragma once

#include "../utility/assert.h"

#include <utility>
#include <type_traits>

#ifdef __cpp_constexpr_dynamic_alloc
#define KTL_CONSTEXPR constexpr
#else // __cpp_constexpr_dynamic_alloc
#define KTL_CONSTEXPR
#endif // __cpp_constexpr_dynamic_alloc

namespace ktl
{
#ifdef KTL_DEBUG_ASSERT
	template<typename T>
	class out
	{
	public:
		KTL_CONSTEXPR out(T& value) noexcept :
			m_Value(value),
			m_Constructed(false) {}

		out(const out&) = delete;

		out(out&&) = delete;

		KTL_CONSTEXPR ~out()
		{
			KTL_ASSERT(m_Constructed);
		}

		template<typename... Ts>
		KTL_CONSTEXPR void construct(Ts&&... args)
			noexcept(std::is_nothrow_destructible_v<T> && std::is_nothrow_constructible_v<T, Ts...>)
		{
			m_Value.~T();

			new(&m_Value) T(std::forward<Ts>(args) ...);

			m_Constructed = true;
		}

		KTL_CONSTEXPR T* operator->() noexcept { return &m_Value; }

		KTL_CONSTEXPR T& operator*() noexcept { return m_Value; }

	private:
		T& m_Value;
		bool m_Constructed;
	};
#else
	template<typename T>
	class out
	{
	public:
		KTL_CONSTEXPR out(T& value) noexcept :
			m_Value(value) {}

		out(const out&) = delete;

		out(out&&) = delete;

		template<typename... Ts>
		KTL_CONSTEXPR void construct(Ts&&... args)
			noexcept(std::is_nothrow_destructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			m_Value.~T();

			new(&m_Value) T(std::forward<Ts>(args) ...);
		}

		KTL_CONSTEXPR T* operator->() noexcept { return &m_Value; }

		KTL_CONSTEXPR T& operator*() noexcept { return m_Value; }

	private:
		T& m_Value;
	};
#endif

	/**
	 * @brief Adds at least an extra mov because a new object is created
	*/
	template<typename T>
	class move
	{
	public:
		constexpr move(T&& value) noexcept :
			m_Value(std::move(value)) {}

		constexpr move(T& value) noexcept :
			m_Value(std::move(value)) {}

		move(const move&) = delete;

		move(move&&) = delete;

		constexpr T* operator->() noexcept { return &m_Value; }

		constexpr T&& operator*() noexcept { return m_Value; }

		constexpr operator T&&() noexcept { return std::move(m_Value); }

	private:
		T&& m_Value;
	};

	/**
	 * @brief Passes by const or const reference depending on size
	*/
	template<typename T>
	using in = std::conditional_t<sizeof(T) < 16, std::add_const_t<T>, std::add_lvalue_reference_t<std::add_const_t<T>>>;

	/**
	 * @brief Passes by reference
	*/
	template<typename T>
	using inout = std::add_lvalue_reference_t<T>;

	/**
	 * @brief Passes by copying
	*/
	template<typename T>
	using copy = T;
}