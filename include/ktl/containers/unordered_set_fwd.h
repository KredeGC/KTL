#pragma once

#include <memory>
#include <functional>
#include <utility>

namespace ktl
{
	template<typename T, typename Hash = std::hash<T>, typename Equals = std::equal_to<T>, typename Alloc = std::allocator<T>>
	class unordered_set;
}