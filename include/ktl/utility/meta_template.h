#pragma once

#include <type_traits>

namespace ktl
{
	// value_type
	template<typename Alloc, typename = void>
	struct has_value_type : std::true_type { };

	template<typename T>
	struct has_value_type<T, std::void_t<typename T::value_type>> : std::false_type { };

	// max_size()
	template<typename Alloc, typename = void>
	struct has_no_max_size : std::true_type { };

	template<typename Alloc>
	struct has_no_max_size<Alloc, std::void_t<decltype(std::declval<Alloc&>().max_size())>> : std::false_type {};

	// construct(T*, Args&&...)
	template<typename Void, typename... Types>
	struct has_no_construct : std::true_type {};

	template<typename Alloc, typename Ptr, typename... Args>
	struct has_no_construct<std::void_t<decltype(std::declval<Alloc&>().construct(std::declval<Ptr>(), std::declval<Args>()...))>, Alloc, Ptr, Args...> : std::false_type {};

	// destroy(T*)
	template<typename Alloc, typename Ptr, typename = void>
	struct has_no_destroy : std::true_type {};

	template<typename Alloc, typename Ptr>
	struct has_no_destroy<Alloc, Ptr, std::void_t<decltype(std::declval<Alloc&>().destroy(std::declval<Ptr>()))>> : std::false_type {};

	// owns(void*)
	template<typename Alloc, typename = void>
	struct has_no_owns : std::true_type { };

	template<typename Alloc>
	struct has_no_owns<Alloc, std::void_t<decltype(std::declval<Alloc&>().owns(std::declval<void*>()))>> : std::false_type {};
}