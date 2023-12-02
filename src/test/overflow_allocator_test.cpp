#include "shared/allocation_utility.h"
#include "shared/binary_heap_utility.h"
#include "shared/test.h"
#include "shared/unordered_map_utility.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/overflow.h"
#include "ktl/allocators/segragator.h"
#include "ktl/allocators/shared.h"
#include "ktl/allocators/stack_allocator.h"

#include "ktl/containers/binary_heap.h"
#include "ktl/containers/trivial_vector.h"

#include <iostream>
#include <sstream>

// Naming scheme: test_overflow_[Alloc]_[Container]_[Type]
// Contains tests that relate directly to the ktl::overflow_allocator

namespace ktl::test::overflow_allocator
{
    template<typename Fn>
    void assert_no_overflow(Fn func)
    {
        std::stringbuf stringBuffer;
        std::ostream stringOut(&stringBuffer);

        // Force it into it's own scope whether it's inlined or not
        {
            func(stringOut);
        }

        std::cerr << stringBuffer.str();

        KTL_TEST_ASSERT(stringBuffer.str().empty());
    }

    KTL_ADD_TEST(test_cascading_linear_raw_allocate)
    {
        assert_no_overflow([](std::ostream& stringOut)
        {
            stack<4096> block;
            overflow<stack_allocator<4096>> alloc(stringOut, block);
            assert_raw_allocate_deallocate<2, 4, 8, 16, 32, 64>(alloc);
        });
    }

    KTL_ADD_TEST(test_overflow_stack_unordered_double)
    {
        assert_no_overflow([](std::ostream& stringOut)
        {
            stack<4096> block;
            type_overflow_allocator<double, stack_allocator<4096>> alloc(stringOut, block);
            assert_unordered_values<double>(alloc);
        });
    }

    KTL_ADD_TEST(test_overflow_linear_unordered_double)
    {
        assert_no_overflow([](std::ostream& stringOut)
        {
            type_overflow_allocator<double, linear_allocator<4096>> alloc(stringOut);
            assert_unordered_values<double>(alloc);
        });
    }

    KTL_ADD_TEST(test_overflow_linear_trivial_vector_double)
    {
        assert_no_overflow([](std::ostream& stringOut)
        {
            type_shared_overflow_allocator<double, linear_allocator<4096>> alloc(stringOut);
            trivial_vector<double, type_shared_overflow_allocator<double, linear_allocator<4096>>> vec(alloc);
            assert_vector_values<double>(vec);
        });
    }

    KTL_ADD_TEST(test_overflow_linear_binary_heap_double)
    {
        assert_no_overflow([](std::ostream& stringOut)
        {
            assert_binary_heap_min_max<double, type_shared_overflow_allocator<double, linear_allocator<4096>>>(stringOut);
        });
    }

    KTL_ADD_TEST(test_overflow_segragator_construct)
    {
        assert_no_overflow([](std::ostream& stringOut)
        {
            type_segragator_allocator<double,
                32,
                    overflow<linear_allocator<4096>>,
                    linear_allocator<4096>> alloc(stringOut);
            
            double* p = alloc.allocate(1);

            alloc.construct(p, 4.20);

            KTL_TEST_ASSERT(*p == 4.20);

            alloc.destroy(p);

            alloc.deallocate(p, 1);
        });
    }
}