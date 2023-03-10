#pragma once

#include <cstddef>

namespace ktl
{
	// Wrapper class for making allocator thread-safe
	template<typename Alloc>
	class threaded;
}