#pragma once

#include "shared_fwd.h"
#include "threaded_fwd.h"
#include "type_allocator_fwd.h"

#include <cstddef>
#include <ostream>

namespace ktl
{
	// debug
	template<typename Alloc, typename Container>
	class debug;

	/**
	 * @brief Shorthand for a typed overflow allocator
	*/
	template<typename T, typename Alloc, typename Container>
	using type_debug_allocator = type_allocator<T, debug<Alloc, Container>>;

	/**
	 * @brief Shorthand for a typed, ref-counted overflow allocator
	*/
	template<typename T, typename Alloc, typename Container>
	using type_shared_debug_allocator = type_allocator<T, shared<debug<Alloc, Container>>>;
}