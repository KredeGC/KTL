#pragma once

#include <atomic>
#include <cstddef>

namespace ktl
{
	// Wrapper class for an integer with std::atomic syntax
	template<typename T>
	class notomic;

	// Wrapper class for making allocator ref-counted
	template<typename Alloc, template<typename> typename Atomic = notomic>
	class shared;

	// Wrapper class for making allocator atomic-ref-counted
	template<typename Alloc>
	using atomic_shared = shared<Alloc, std::atomic>;
}