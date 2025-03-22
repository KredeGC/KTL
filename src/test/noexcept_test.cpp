#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/cascading.h"
#include "ktl/allocators/debug.h"
#include "ktl/allocators/fallback.h"
#include "ktl/allocators/freelist.h"
#include "ktl/allocators/global.h"
#include "ktl/allocators/null_allocator.h"
#include "ktl/allocators/overflow.h"
#include "ktl/allocators/reference.h"
#include "ktl/allocators/segragator.h"
#include "ktl/allocators/shared.h"
#include "ktl/allocators/threaded.h"

#include <sstream>
#include <vector>

// Naming scheme: test_nothrow_allocator_[ID]
// Contains static tests for throwing and non-throwing allocators

namespace ktl::test::nothrow_allocator
{
    class throwing_allocator
    {
	public:
		throwing_allocator() {}

		throwing_allocator(const throwing_allocator&) {}

		throwing_allocator(throwing_allocator&&) = default;

		throwing_allocator& operator=(const throwing_allocator&) {}

		throwing_allocator& operator=(throwing_allocator&&) = default;

		bool operator==(const throwing_allocator& rhs) const
		{
			return true;
		}

		bool operator!=(const throwing_allocator& rhs) const
		{
			return false;
		}

		void* allocate(size_t n)
		{
			return nullptr;
		}

		void deallocate(void* p, size_t n)
		{
			KTL_ASSERT(p == nullptr);
		}

		bool owns(void* p) const
		{
			return p == nullptr;
		}
    };

	template<typename Alloc, bool NoThrow>
	constexpr void test_allocator_nothrow()
	{
		if constexpr (std::is_default_constructible_v<Alloc>)
			static_assert(std::is_nothrow_default_constructible_v<Alloc> == NoThrow);
		if constexpr (std::is_copy_constructible_v<Alloc>)
			static_assert(std::is_nothrow_copy_constructible_v<Alloc> == NoThrow);
		if constexpr (std::is_copy_assignable_v<Alloc>)
			static_assert(std::is_nothrow_copy_assignable_v<Alloc> == NoThrow);

		static_assert(detail::has_nothrow_allocate_v<Alloc> == NoThrow);
		static_assert(detail::has_nothrow_deallocate_v<Alloc> == NoThrow);
		static_assert(detail::has_nothrow_equal_v<Alloc> == NoThrow);
		static_assert(detail::has_nothrow_not_equal_v<Alloc> == NoThrow);
		if constexpr (detail::has_max_size_v<Alloc>)
			static_assert(detail::has_nothrow_max_size_v<Alloc> == NoThrow);
		if constexpr (detail::has_owns_v<Alloc>)
			static_assert(detail::has_nothrow_owns_v<Alloc> == NoThrow);
	}

	KTL_ADD_TEST(test_nothrow_allocator)
	{
		struct debug_info
		{
			std::string_view File;
			uint_least32_t Line;
			size_t Size;
		};

		// Non-throwing
		test_allocator_nothrow<null_allocator, true>();
		test_allocator_nothrow<cascading<null_allocator>, true>();
		test_allocator_nothrow<debug<null_allocator, std::vector<debug_info>>, true>();
		test_allocator_nothrow<fallback<null_allocator, null_allocator>, true>();
		test_allocator_nothrow<global<null_allocator>, true>();
		test_allocator_nothrow<overflow<null_allocator>, true>();
		test_allocator_nothrow<reference<null_allocator>, true>();
		test_allocator_nothrow<segragator<16, null_allocator, null_allocator>, true>();
		test_allocator_nothrow<shared<null_allocator>, true>();
		test_allocator_nothrow<threaded<null_allocator>, true>();

		// Throwing
		test_allocator_nothrow<throwing_allocator, false>();
		test_allocator_nothrow<debug<null_allocator, std::vector<debug_info>>, true>();
		test_allocator_nothrow<fallback<throwing_allocator, throwing_allocator>, false>();
		test_allocator_nothrow<overflow<throwing_allocator>, false>();
		test_allocator_nothrow<segragator<16, throwing_allocator, throwing_allocator>, false>();
		test_allocator_nothrow<threaded<throwing_allocator>, false>();

		// Composite of throwing and non-throwing
		test_allocator_nothrow<fallback<throwing_allocator, global<null_allocator>>, false>();
		test_allocator_nothrow<segragator<16, throwing_allocator, null_allocator>, false>();
	}
}