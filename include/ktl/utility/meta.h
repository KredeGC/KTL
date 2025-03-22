#pragma once

#include "source_location.h"

#include <type_traits>

namespace ktl::detail
{
	// has value_type
	template<typename Alloc, typename = void>
	struct has_no_value_type : std::true_type {};

	template<typename Alloc>
	struct has_no_value_type<Alloc, std::void_t<typename Alloc::value_type>> : std::false_type {};

	template<typename Alloc>
	constexpr bool has_no_value_type_v = has_no_value_type<Alloc, void>::value;

	// get size_type
	template<typename Alloc, typename = void>
	struct get_size_type
	{
		using type = size_t;
	};

	template<typename Alloc>
	struct get_size_type<Alloc, std::void_t<typename Alloc::size_type>>
	{
		using type = typename Alloc::size_type;
	};

	template<typename Alloc, typename = void>
	using get_size_type_t = typename get_size_type<Alloc, void>::type;

	// has allocate(size_t)
	template<typename Alloc, typename = void>
	struct has_plain_allocate : std::false_type {};

	template<typename Alloc>
	struct has_plain_allocate<Alloc, std::void_t<decltype(std::declval<Alloc&>().allocate(std::declval<size_t>()))>> : std::true_type {};

	template<typename Alloc>
	constexpr bool has_plain_allocate_v = has_plain_allocate<Alloc>::value;

	// has construct(T*, Args&&...)
	template<typename Void, typename... Types>
	struct has_construct : std::false_type {};

	template<typename Alloc, typename... Args>
	struct has_construct<std::void_t<decltype(std::declval<Alloc&>().construct(std::declval<Args>()...))>, Alloc, Args...> : std::true_type {};

	template<typename Alloc, typename... Args>
	constexpr bool has_construct_v = has_construct<void, Alloc, Args...>::value;

	// has destroy(T*)
	template<typename Alloc, typename Ptr, typename = void>
	struct has_destroy : std::false_type {};

	template<typename Alloc, typename Ptr>
	struct has_destroy<Alloc, Ptr, std::void_t<decltype(std::declval<Alloc&>().destroy(std::declval<Ptr>()))>> : std::true_type {};

	template<typename Alloc, typename Ptr>
	constexpr bool has_destroy_v = has_destroy<Alloc, Ptr, void>::value;

	// has max_size()
	template<typename Alloc, typename = void>
	struct has_max_size : std::false_type {};

	template<typename Alloc>
	struct has_max_size<Alloc, std::void_t<decltype(std::declval<Alloc&>().max_size())>> : std::true_type {};

	template<typename Alloc>
	constexpr bool has_max_size_v = has_max_size<Alloc, void>::value;

	// has owns(void*)
	template<typename Alloc, typename = void>
	struct has_owns : std::false_type {};

	template<typename Alloc>
	struct has_owns<Alloc, std::void_t<decltype(std::declval<Alloc&>().owns(std::declval<void*>()))>> : std::true_type {};

	template<typename Alloc>
	constexpr bool has_owns_v = has_owns<Alloc, void>::value;



	// has allocate(size_t) noexcept
	template<typename Alloc, bool>
	struct has_nothrow_allocate : std::false_type {};

	template<typename Alloc>
	struct has_nothrow_allocate<Alloc, true>
		: std::bool_constant<noexcept(std::declval<Alloc&>().allocate(std::declval<size_t>()))> {};

	template<typename Alloc>
	struct has_nothrow_allocate<Alloc, false>
		: std::bool_constant<noexcept(std::declval<Alloc&>().allocate(std::declval<size_t>(), std::declval<source_location>()))> {};

	template<typename Alloc>
	constexpr bool has_nothrow_allocate_v = has_nothrow_allocate<Alloc, has_plain_allocate_v<Alloc>>::value;

	// has deallocate(void*, size_t) noexcept
	template<typename Alloc>
	constexpr bool has_nothrow_deallocate_v = noexcept(std::declval<Alloc&>().deallocate(std::declval<void*>(), std::declval<size_t>()));

	// has T& == T& noexcept
	template<typename T>
	constexpr bool has_nothrow_equal_v = noexcept(std::declval<T&>() == std::declval<T&>());

	// has T& != T& noexcept
	template<typename T>
	constexpr bool has_nothrow_not_equal_v = noexcept(std::declval<T&>() == std::declval<T&>());

	// has construct(T*, Args&&...) noexcept
	template<typename Void, typename... Types>
	struct has_nothrow_construct : std::false_type {};

	template<typename Alloc, typename... Args>
	struct has_nothrow_construct<std::enable_if_t<has_construct_v<Alloc, Args...>>, Alloc, Args...>
		: std::bool_constant<noexcept(std::declval<Alloc&>().construct(std::declval<Args>()...))> {};

	template<typename Alloc, typename... Args>
	constexpr bool has_nothrow_construct_v = has_nothrow_construct<void, Alloc, Args...>::value;

	// has destroy(T*) noexcept
	template<typename Alloc, typename Ptr, typename = void>
	struct has_nothrow_destroy : std::false_type {};

	template<typename Alloc, typename Ptr>
	struct has_nothrow_destroy<Alloc, Ptr, std::enable_if_t<has_destroy_v<Alloc, Ptr>>>
		: std::bool_constant<noexcept(std::declval<Alloc&>().destroy(std::declval<Ptr>()))> {};

	template<typename Alloc, typename Ptr>
	constexpr bool has_nothrow_destroy_v = has_nothrow_destroy<Alloc, Ptr, void>::value;

	// has max_size() noexcept
	template<typename Alloc, typename = void>
	struct has_nothrow_max_size : std::false_type {};

	template<typename Alloc>
	struct has_nothrow_max_size<Alloc, std::enable_if_t<has_max_size_v<Alloc>>>
		: std::bool_constant<noexcept(std::declval<Alloc&>().max_size())> {};

	template<typename Alloc>
	constexpr bool has_nothrow_max_size_v = has_nothrow_max_size<Alloc, void>::value;

	// has owns(void*) noexcept
	template<typename Alloc, typename = void>
	struct has_nothrow_owns : std::false_type {};

	template<typename Alloc>
	struct has_nothrow_owns<Alloc, std::enable_if_t<has_owns_v<Alloc>>>
		: std::bool_constant<noexcept(std::declval<Alloc&>().owns(std::declval<void*>()))> {};

	template<typename Alloc>
	constexpr bool has_nothrow_owns_v = has_nothrow_owns<Alloc, void>::value;



	template<typename Alloc>
	void* allocate(Alloc& alloc, size_t n, const source_location source) noexcept(false)
	{
		if constexpr (has_plain_allocate_v<Alloc>)
			return alloc.allocate(n);
		else
			return alloc.allocate(n, source);
	}
}