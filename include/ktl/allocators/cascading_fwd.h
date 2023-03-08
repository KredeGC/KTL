#pragma once

#include "shared_fwd.h"
#include "threaded_fwd.h"
#include "type_allocator_fwd.h"

#include <cstddef>

namespace ktl
{
    // cascading
	template<typename Alloc>
	class cascading;


	template<typename T, typename Alloc>
	using type_cascading_allocator = type_allocator<T, cascading<Alloc>>;
}