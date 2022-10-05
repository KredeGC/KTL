#include "shared/allocation_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/test.h"

#include "ktl/ktl_fwd.h"

#include "ktl/containers/binary_heap.h"
#include "ktl/allocators/freelist_allocator.h"
#include "ktl/allocators/overflow_allocator.h"

#include <sstream>

// Naming scheme: test_overflow_[Alloc]_[Container]_[Type]
// Contains tests that relate directly to the mallocator

namespace ktl
{
    static std::stringbuf stringBuffer;
    static std::ostream stringOut(&stringBuffer);

    KTL_ADD_TEST(test_overflow_freelist_unordered_double)
    {
        stringBuffer = std::stringbuf();

        {
            freelist<4096> list;
            type_overflow_allocator<double, freelist_allocator<4096>, stringOut> alloc({ list });
            assert_unordered_values<double>(alloc);
        }

        std::cout << stringBuffer.str();

        KTL_ASSERT(stringBuffer.str().empty());
    }

    KTL_ADD_TEST(test_overflow_freelist_binary_heap_double)
    {
        stringBuffer = std::stringbuf();

        {
            freelist<4096> block;
            type_overflow_allocator<double, freelist_allocator<4096>, stringOut> alloc({ block });
            assert_binary_heap<double>(3, alloc);
        }

        std::cout << stringBuffer.str();

        KTL_ASSERT(stringBuffer.str().empty());
    }
}