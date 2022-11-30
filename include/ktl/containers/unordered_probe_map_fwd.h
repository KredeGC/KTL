#pragma once

#include <memory>
#include <functional>

namespace ktl
{
	template<typename K, typename V, typename Hash = std::hash<K>, typename Equals = std::equal_to<K>, typename Alloc = std::allocator<V>>
	class unordered_probe_map;
}