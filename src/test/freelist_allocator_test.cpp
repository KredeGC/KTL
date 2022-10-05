#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/allocators/freelist_allocator.h"

namespace ktl
{
    KTL_ADD_TEST(test_freelist_allocator_double)
    {
        double values[] = {
                42.5,
                81.3,
                384.6,
                182.1,
                99.9
        };

        freelist<4096> block;
        type_freelist_allocator<double, 4096> alloc(block);

        assert_allocate_unordered(alloc, values);
    }

    KTL_ADD_TEST(test_freelist_allocator_packed)
    {
        int forRef1 = 42;
        int forRef2 = 9;

        packed_t values[] = {
            { &forRef1, &forRef2, 55, 'm' },
            { &forRef1, &forRef2, 982, 'c' },
            { &forRef1, &forRef2, 761, 'p' },
            { &forRef1, &forRef2, 666, 'd' },
            { &forRef1, &forRef2, 40000, 'a' }
        };

        freelist<4096> block;
        type_freelist_allocator<packed_t, 4096> alloc(block);

        assert_allocate_unordered(alloc, values);
    }
}