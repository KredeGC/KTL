#pragma once

#include "type_allocator_fwd.h"

#include <cstddef>

namespace ktl
{
	// mallocator
	class mallocator;

	/**
	 * @brief Shorthand for a typed mallocator
	*/
	template<typename T>
	using type_mallocator = type_allocator<T, mallocator>;
}