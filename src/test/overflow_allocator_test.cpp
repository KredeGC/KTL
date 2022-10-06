#include "shared/allocation_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/test.h"

#include "ktl/ktl_fwd.h"

#include "ktl/containers/binary_heap.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/overflow_allocator.h"
#include "ktl/allocators/stack_allocator.h"

#include <sstream>

// Naming scheme: test_overflow_[Alloc]_[Container]_[Type]
// Contains tests that relate directly to the ktl::overflow_allocator

namespace ktl
{
    KTL_ADD_HEADER();

    static std::stringbuf stringBuffer;
    static std::ostream stringOut(&stringBuffer);

    template<typename Fn>
    void assert_no_overflow(Fn func)
    {
        stringBuffer = std::stringbuf();

        // Force it into it's own scope whether it's inlined or not
        {
            func();
        }

        std::cout << stringBuffer.str();

        KTL_ASSERT(stringBuffer.str().empty());
    }

    KTL_ADD_TEST(test_overflow_stack_unordered_double)
    {
        assert_no_overflow([]()
        {
            stack<4096> block;
            type_overflow_allocator<double, stack_allocator<4096>, stringOut> alloc({ block });
            assert_unordered_values<double>(alloc);
        });
    }

    KTL_ADD_TEST(test_overflow_freelist_unordered_double)
    {
        assert_no_overflow([]()
        {
            arena<4096> block;
            type_overflow_allocator<double, pre_allocator<4096>, stringOut> alloc({ block });
            assert_unordered_values<double>(alloc);
        });
    }

    KTL_ADD_TEST(test_overflow_freelist_binary_heap_double)
    {
        assert_no_overflow([]()
        {
            arena<4096> block;
            type_overflow_allocator<double, pre_allocator<4096>, stringOut> alloc({ block });
            assert_binary_heap<double>(3, alloc);
        });
    }
}