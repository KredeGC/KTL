#pragma once

#include <cstddef>

namespace ktl
{
	template<typename PtrT, size_t Bits, size_t Alignment = alignof(PtrT)>
	class packed_ptr;
}