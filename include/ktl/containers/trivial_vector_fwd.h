#pragma once

#include <memory>

namespace ktl
{
	template<typename T, typename Alloc = std::allocator<T>>
	class trivial_vector;
}