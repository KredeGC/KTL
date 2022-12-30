#pragma once

#include "allocators/cascading_fwd.h"
#include "allocators/fallback_fwd.h"
#include "allocators/freelist_fwd.h"
#include "allocators/linear_allocator_fwd.h"
#include "allocators/linked_fwd.h"
#include "allocators/mallocator_fwd.h"
#include "allocators/overflow_fwd.h"
#include "allocators/segragator_fwd.h"
#include "allocators/stack_allocator_fwd.h"

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
    class type_allocator;

	// null_allocator
	class null_allocator;
}