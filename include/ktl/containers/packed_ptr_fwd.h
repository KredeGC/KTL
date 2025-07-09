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
			if constexpr (std::is_unsigned_v<T> || std::is_enum_v<T>)
				return static_cast<T>(0ULL);
			else
				return static_cast<T>(-(1LL << (bits - 1LL)));
		}

		template<typename T>
		constexpr T max_range(size_t bits)
		{
			if constexpr (std::is_unsigned_v<T> || std::is_enum_v<T>)
				return static_cast<T>((1ULL << bits) - 1ULL);
			else
				return static_cast<T>(((1LL << bits) - 1LL) - (1LL << (bits - 1LL)));
		}


		// Function pointer types
		template<typename T, typename = void>
		struct is_function_pointer : std::false_type {};

		template<typename R, typename... Ts>
		struct is_function_pointer<R(*)(Ts...), std::void_t<R(*)(Ts...)>> : std::true_type {};

		template<typename T>
		constexpr bool is_function_pointer_v = is_function_pointer<T>::value;


		// Get underlying type for alignment
		template<typename T, typename = void>
		struct underlying_type
		{
			using type = T;
		};

		// Enum types
		template<typename T>
		struct underlying_type<T, std::enable_if_t<std::is_enum_v<T>>>
		{
			using type = std::underlying_type_t<T>;
		};

		// Function types
		template<typename T>
		struct underlying_type<T, std::enable_if_t<std::is_function_v<T>>>
		{
			using type = uint64_t;
		};

		// Pointer types, but not function pointers
		template<typename T>
		struct underlying_type<T, std::enable_if_t<std::is_pointer_v<T> && !is_function_pointer_v<T>>>
		{
			using type = std::remove_pointer_t<T>;
		};

		template<typename T>
		using underlying_type_t = typename underlying_type<T>::type;
	}

	template<
		typename PtrT,
		typename IntT,
		IntT Min = detail::min_range<IntT>(detail::log2(alignof(detail::underlying_type_t<PtrT>))),
		IntT Max = detail::max_range<IntT>(detail::log2(alignof(detail::underlying_type_t<PtrT>))),
		size_t Alignment = alignof(detail::underlying_type_t<PtrT>)>
	class packed_ptr;
}