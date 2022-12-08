#pragma once

#include <memory>
#include <functional>
#include <utility>

namespace ktl
{
	template<typename K, typename V, typename Hash = std::hash<K>, typename Equals = std::equal_to<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
	class unordered_map;
}