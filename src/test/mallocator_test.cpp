#include "shared/allocation_utility.h"
#include "shared/test.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/mallocator.h"

namespace ktl
{
    KTL_ADD_TEST(test_mallocator_double)
    {
        double values[] = {
                42.5,
                81.3,
                384.6,
                182.1,
                99.9
        };

        type_mallocator<double> alloc;

        assert_allocate_unordered(alloc, values);
    }
}