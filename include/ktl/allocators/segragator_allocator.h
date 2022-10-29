#pragma once

#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Threshold, typename P, typename F>
	class segragator_allocator
	{
	private:
		static_assert(has_value_type<P>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(has_value_type<F>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(!has_construct<P>::value || has_owns<P>::value, "The primary allocator is required to have an 'owns(void*)' method, if it has a construct(void*, Args...) method");

	public:
		typedef typename get_size_type<P>::type size_type;


	};
}