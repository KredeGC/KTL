#pragma once

#include "shared_fwd.h"
#include "type_allocator_fwd.h"

namespace ktl
{
	// Wrapper class for making allocator thread-safe
	template<typename Alloc>
	class threaded;

	/**
	 * @brief Shorthand for a typed, ref-counted thread-safe allocator
	*/
	template<typename T, typename Alloc>
	using type_shared_threaded = type_allocator<T, atomic_shared<threaded<Alloc>>>;
}