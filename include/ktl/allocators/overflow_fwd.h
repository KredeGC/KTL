#pragma once

#include "shared_fwd.h"
#include "threaded_fwd.h"
#include "type_allocator_fwd.h"

#include <cstddef>
#include <ostream>

namespace ktl
{
    // overflow
	template<typename Alloc, typename Stream = std::ostream>
	class overflow;

	/**
	 * @brief Shorthand for a typed overflow allocator
	*/
	template<typename T, typename Alloc, typename Stream = std::ostream>
	using type_overflow_allocator = type_allocator<T, overflow<Alloc, Stream>>;

	/**
	 * @brief Shorthand for a typed, ref-counted overflow allocator
	*/
	template<typename T, typename Alloc, typename Stream = std::ostream>
	using type_shared_overflow_allocator = type_allocator<T, shared<overflow<Alloc, Stream>>>;

	/**
	 * @brief Shorthand for a typed, thread-safe, ref-counted overflow allocator
	*/
	template<typename T, typename Alloc, typename Stream = std::ostream>
	using type_threaded_overflow_allocator = type_allocator<T, threaded<overflow<Alloc, Stream>>>;
}