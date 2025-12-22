#pragma once

#include "trivial_vector_fwd.h"

#include <memory>
#include <vector>

namespace ktl
{
	template<typename T, typename Vec>
	class small_vector;

	template<typename T, typename Alloc = std::allocator<T>>
	using small_trivial_vector = small_vector<T, ktl::trivial_vector<T, Alloc>>;

	template<typename T, typename Alloc = std::allocator<T>>
	using small_std_vector = small_vector<T, std::vector<T, Alloc>>;
}