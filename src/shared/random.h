#pragma once

#include <algorithm>
#include <random>

namespace ktl
{
	inline std::random_device rd;
	inline std::mt19937 random_generator(rd());
}