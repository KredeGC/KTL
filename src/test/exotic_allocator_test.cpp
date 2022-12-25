#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/cascading.h"
#include "ktl/allocators/fallback.h"
#include "ktl/allocators/freelist.h"
#include "ktl/allocators/list_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/overflow.h"
#include "ktl/allocators/segragator.h"
#include "ktl/allocators/stack_allocator.h"

#include <sstream>

// Naming scheme: test_exotic_allocator_[ID]
// Contains tests of all sorts for exotic composable allocators

namespace ktl::test::exotic_allocator
{
    static std::stringbuf stringBuffer;
    static std::ostream stringOut(&stringBuffer);

    KTL_ADD_TEST(test_exotic_allocator_0)
    {
        // TODO: More tests with exotic allocator arrangements
        stack<1024> block;
        type_segragator_allocator<double, 32, stack_allocator<1024>, cascading<list_allocator<1024>>> alloc2({ block });
    }

    KTL_ADD_TEST(test_exotic_allocator_1)
    {
        // Create the allocator from some 16kb buffer and straight malloc
        type_fallback_allocator<double, list_allocator<16384>, mallocator> alloc;
        // Allocate and deallocate 3 doubles
        double* p1 = alloc.allocate(3);
        alloc.deallocate(p1, 3);
        // Allocate and deallocate 4096 doubles, which should be handled by malloc
        double* p2 = alloc.allocate(4096);
        alloc.deallocate(p2, 4096);

        KTL_TEST_ASSERT(p1 != p2);
    }

    KTL_ADD_TEST(test_exotic_allocator_2)
    {
        stringBuffer = std::stringbuf();

        {
            // Create the allocator with std::cerr
            type_overflow_allocator<double, mallocator, stringOut> alloc;
            // Allocate and deallocate 1 double
            double* p = alloc.allocate(1);
            // Write to the address before what was allocated, which is illegal
            *(p - 1) = 32;
            // When it deallocates it should give a message in the standard error stream
            alloc.deallocate(p, 1);
        }

        KTL_TEST_ASSERT(!stringBuffer.str().empty());
    }

    KTL_ADD_TEST(test_exotic_allocator_3)
    {
        // Create the allocator from a combination of a cascading 8kb pre allocator and malloc
        // Anything smaller than 8kb should use the cascading pre allcoator, while anything larger should use malloc
        type_segragator_allocator<double, 8192, cascading<list_allocator<8192>>, mallocator> alloc;
        // Allocate 1024 doubles
        double* p1 = alloc.allocate(1024);
        // Allocate another 1024 doubles, which should force the allocator to create a new pre allocator
        double* p2 = alloc.allocate(1024);
        // Allocate 2048 doubles, which should use malloc instead
        double* p3 = alloc.allocate(2048);
        // Deallocate all of it
        alloc.deallocate(p1, 1024);
        alloc.deallocate(p2, 1024);
        alloc.deallocate(p3, 2048);

        KTL_TEST_ASSERT(p1 != p2);
        KTL_TEST_ASSERT(p2 != p3);
    }

    KTL_ADD_TEST(test_exotic_allocator_4)
    {
        // Create the allocator from a freelist, backed by malloc
        type_segragator_allocator<double, 16, freelist<0, 16, mallocator>, mallocator> alloc;
        // Allocate 1 double and deallocate, so that it ends up in the freelist
        double* p1 = alloc.allocate(1);
        alloc.deallocate(p1, 1);
        // Allocate 2 doubles, which should reuse the previous allocation
        double* p2 = alloc.allocate(2);
        alloc.deallocate(p2, 2);
        // Allocate 4 doubles, which should be handled by the backup malloc
        double* p3 = alloc.allocate(4);
        alloc.deallocate(p3, 4);

        KTL_TEST_ASSERT(p1 == p2);
        KTL_TEST_ASSERT(p2 != p3);
    }
}