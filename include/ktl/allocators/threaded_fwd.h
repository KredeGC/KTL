#pragma once

#include "shared_fwd.h"

#include <atomic>
#include <cstddef>

namespace ktl
{
	// Wrapper class for making allocator thread-safe
	template<typename Alloc>
	class threaded;

	/**
	 * @brief Typedef for wrapping an STL-compliant thread-safe allocoator
	*/
	template<typename Alloc>
	using shared_threaded = shared<threaded<Alloc>, std::atomic<size_t>>;
}