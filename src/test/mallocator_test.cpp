#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/mallocator.h"

// Naming scheme: test_mallocator_[Type]
// Contains tests that relate directly to the ktl::mallocator

namespace ktl
{
    KTL_ADD_TEST(test_mallocator_double)
    {
        type_mallocator<double> alloc;
        assert_unordered_values<double>(alloc);
    }

    KTL_ADD_TEST(test_mallocator_packed)
    {
        type_mallocator<packed_t> alloc;
        assert_unordered_values<packed_t>(alloc);
    }
}