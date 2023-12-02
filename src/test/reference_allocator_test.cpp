#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/reference.h"

#include <vector>

// Naming scheme: test_reference_allocator_[Type]
// Contains tests that relate directly to the ktl::reference

namespace ktl::test::reference_allocator
{
    KTL_ADD_TEST(test_reference_allocator_construct)
    {
        ktl::linear_allocator<4096> alloc;

        ktl::reference<ktl::linear_allocator<4096>> ref(alloc);

        ktl::reference<ktl::linear_allocator<4096>> moved_ref(std::move(ref));
    }

    KTL_ADD_TEST(test_reference_allocator_raw_allocate)
    {
        ktl::linear_allocator<4096> alloc;

        ktl::reference<ktl::linear_allocator<4096>> ref(alloc);

        assert_raw_allocate_deallocate(ref);
    }
}