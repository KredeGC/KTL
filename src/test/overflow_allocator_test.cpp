#include "shared/allocation_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/test.h"

#include "ktl/ktl_fwd.h"

#include "ktl/containers/binary_heap.h"
#include "ktl/allocators/freelist_allocator.h"
#include "ktl/allocators/overflow_allocator.h"

#include <sstream>

namespace ktl
{
    static std::stringbuf stringBuffer;
    static std::ostream stringOut(&stringBuffer);

    KTL_ADD_TEST(test_freelist_overflow)
    {
        stringBuffer = std::stringbuf();

        {
            double values[] = {
                42.5,
                81.3,
                384.6,
                182.1,
                99.9
            };

            freelist<4096> list;
            type_overflow_allocator<double, freelist_allocator<4096>, stringOut> alloc({ list });

            assert_allocate_unordered(alloc, values);
        }

        std::cout << stringBuffer.str();

        KTL_ASSERT(stringBuffer.str().empty());
    }

    KTL_ADD_TEST(test_binary_heap_overflow)
    {
        stringBuffer = std::stringbuf();

        {
            constexpr size_t size = 8;

            double values[] = {
                0.0,
                8.0,
                9.0,
                10.0,
                20.0,
                28.0,
                32.0,
                58.0
            };

            freelist<4096> block;
            type_overflow_allocator<double, freelist_allocator<4096>, stringOut> alloc({ block });
            test_binary_heap_min_max<3>(alloc, values, size);
        }

        std::cout << stringBuffer.str();

        KTL_ASSERT(stringBuffer.str().empty());
    }
}