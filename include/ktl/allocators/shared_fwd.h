#pragma once

#include <cstddef>

namespace ktl
{
	// Fake atomic counter
	template<typename T>
	struct notomic;

	// Wrapper class for making allocator ref-counted
	template<typename Alloc, typename Atomic = notomic<size_t>>
	class shared;
}