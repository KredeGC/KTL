#pragma once

#include <cstddef>
#include <type_traits>

namespace ktl
{
	template<typename PtrT, size_t Bits, size_t Alignment = alignof(std::remove_pointer_t<PtrT>)>
	class packed_ptr;
}