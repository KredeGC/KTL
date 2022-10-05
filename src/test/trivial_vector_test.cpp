#include "shared/assert_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_fwd.h"

#include "ktl/containers/trivial_vector.h"

#include "ktl/allocators/freelist_allocator.h"
#include "ktl/allocators/stack_allocator.h"
#include "ktl/allocators/mallocator.h"

// Naming scheme: test_trivial_vector_[Alloc]_[Type]
// Contains tests that relate directly to the ktl::trivial_vector

namespace ktl
{
    KTL_ADD_TEST(test_trivial_vector_freelist_double)
    {
        freelist<4096> block;
        trivial_vector<double, type_freelist_allocator<double, 4096>> vec({ block });
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_trivial_vector_freelist_trivial)
    {
        freelist<4096> block;
        trivial_vector<trivial_t, type_freelist_allocator<trivial_t, 4096>> vec({ block });
        assert_vector_values<trivial_t>(vec);
    }
}