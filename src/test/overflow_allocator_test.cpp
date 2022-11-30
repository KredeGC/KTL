#include "shared/allocation_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/test.h"
#include "shared/unordered_map_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/overflow_allocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

#include "ktl/containers/binary_heap.h"
#include "ktl/containers/unordered_probe_map.h"

#include <iostream>
#include <sstream>

// Naming scheme: test_overflow_[Alloc]_[Container]_[Type]
// Contains tests that relate directly to the ktl::overflow_allocator

namespace ktl::test::overflow_allocator
{
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

        std::cerr << stringBuffer.str();

        KTL_TEST_ASSERT(stringBuffer.str().empty());
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

    KTL_ADD_TEST(test_overflow_pre_unordered_double)
    {
        assert_no_overflow([]()
        {
            type_overflow_allocator<double, pre_allocator<4096>, stringOut> alloc;
            assert_unordered_values<double>(alloc);
        });
    }

    KTL_ADD_TEST(test_overflow_pre_binary_heap_double)
    {
        assert_no_overflow([]()
        {
            type_overflow_allocator<double, pre_allocator<4096>, stringOut> alloc;
            assert_binary_heap<double>(3, alloc);
        });
    }

    KTL_ADD_TEST(test_overflow_pre_unordered_probe_map_string_double)
    {
        using Alloc = type_overflow_allocator<double, pre_allocator<4096>, stringOut>;

        assert_no_overflow([]()
        {
            Alloc alloc;
            ktl::unordered_probe_map<std::string, double, std::hash<std::string>, std::equal_to<std::string>, Alloc> map(alloc);
            assert_unordered_map_values<double>(map);
        });
    }

    KTL_ADD_TEST(test_overflow_pre_unordered_probe_map_string_complex)
    {
        using Alloc = type_overflow_allocator<complex_t, pre_allocator<4096>, stringOut>;

        assert_no_overflow([]()
        {
            Alloc alloc;
            ktl::unordered_probe_map<std::string, complex_t, std::hash<std::string>, std::equal_to<std::string>, Alloc> map(alloc);
            assert_unordered_map_values<complex_t>(map);
        });
    }
}