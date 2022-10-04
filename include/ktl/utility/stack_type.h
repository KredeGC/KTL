#pragma once

#include "alignment_utility.h"

namespace ktl
{
	template<size_t Size = 4096>
	struct stack
	{
		char Data[Size + ALIGNMENT - 1];
	};
}