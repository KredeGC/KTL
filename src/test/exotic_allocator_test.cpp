#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/cascading.h"
#include "ktl/allocators/debug.h"
#include "ktl/allocators/fallback.h"
#include "ktl/allocators/freelist.h"
#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/overflow.h"
#include "ktl/allocators/segragator.h"
#include "ktl/allocators/stack_allocator.h"

#include <sstream>
#include <vector>

// Naming scheme: test_exotic_allocator_[ID]
// Contains tests of all sorts for exotic composable allocators

namespace ktl::test::exotic_allocator
{
    KTL_ADD_TEST(test_exotic_allocator_0)
    {
        // TODO: More tests with exotic allocator arrangements
        stack<1024> block;
        segragator<32, stack_allocator<1024>, cascading<linear_allocator<1024>>> alloc(block);

        segragator<32, stack_allocator<1024>, cascading<linear_allocator<1024>>> alloc2(std::move(alloc));
    }

    KTL_ADD_TEST(test_exotic_allocator_1)
    {
        // Create the allocator from some 8kb buffer and straight malloc
        type_fallback_allocator<double, linear_allocator<8192>, mallocator> alloc;
        // Allocate and deallocate 3 doubles
        double* p1 = alloc.allocate(3);
        alloc.deallocate(p1, 3);
        // Allocate and deallocate 2048 doubles, which should be handled by malloc
        double* p2 = alloc.allocate(2048);
        alloc.deallocate(p2, 2048);

        KTL_TEST_ASSERT(p1 != p2);
    }

    KTL_ADD_TEST(test_exotic_allocator_2)
    {
        std::stringbuf stringBuffer;
        std::ostream stringOut(&stringBuffer);

        {
            // Create the allocator with std::cerr
            type_overflow_allocator<double, mallocator> alloc(stringOut);
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
        // Create the allocator from a combination of a cascading 8kb linear allocator and malloc
        // Anything smaller than 8kb should use the cascading linear allocator, while anything larger should use malloc
        type_segragator_allocator<double, 8192, cascading<linear_allocator<8192>>, mallocator> alloc;
        // Allocate 1024 doubles
        double* p1 = alloc.allocate(1024);
        // Allocate another 1024 doubles, which should force the allocator to create a new linear allocator
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
    
    // Just a shorthand for writing freelists
    template<size_t Max>
    using FList = freelist<0, Max, mallocator>;
    
    KTL_ADD_TEST(test_exotic_allocator_5)
    {
        // Create the allocator from various thresholded freelists, backed by a cascading linear allocator and malloc
        using Alloc = segragator_builder_max<
            FList<8>,
            threshold<8>,
            FList<128>,
            threshold<128>,
            FList<512>,
            threshold<512>,
            FList<1024>,
            threshold<1024>,
            cascading<linear_allocator<4096>>,
            threshold<4096>,
            mallocator>;
        
        Alloc alloc;
        // Allocate and deallocate 256 bytes, which should use the third freelist
        void* p1 = alloc.allocate(256);
        alloc.deallocate(p1, 256);
        // Allocate and deallocate 256 bytes, which should reuse the previous allocation
        void* p2 = alloc.allocate(256);
        alloc.deallocate(p2, 256);
        // Allocate and deallocate 2048 bytes, which should use the cascading linear allocator
        void* p3 = alloc.allocate(2048);
        alloc.deallocate(p3, 2048);
        
        KTL_TEST_ASSERT(p1 == p2);
        KTL_TEST_ASSERT(p2 != p3);
    }

    KTL_ADD_TEST(test_exotic_allocator_6)
    {
        stack<1024> block1;
        stack<1024> block2;

        // Create the allocator from various thresholded freelists, backed by a cascading linear allocator and malloc
        using Alloc = segragator_builder_max<
            stack_allocator<1024>,
            threshold<8>,
            stack_allocator<1024>,
            threshold<128>,
            FList<512>,
            threshold<512>,
            FList<1024>,
            threshold<1024>,
            cascading<linear_allocator<4096>>,
            threshold<4096>,
            mallocator>;

        Alloc alloc(std::forward_as_tuple(std::forward_as_tuple(block1), std::forward_as_tuple(block2)), std::make_tuple());

        // Allocate and deallocate 256 bytes, which should use the third freelist
        void* p1 = alloc.allocate(256);
        alloc.deallocate(p1, 256);
        // Allocate and deallocate 256 bytes, which should reuse the previous allocation
        void* p2 = alloc.allocate(256);
        alloc.deallocate(p2, 256);
        // Allocate and deallocate 2048 bytes, which should use the cascading linear allocator
        void* p3 = alloc.allocate(2048);
        alloc.deallocate(p3, 2048);

        KTL_TEST_ASSERT(p1 == p2);
        KTL_TEST_ASSERT(p2 != p3);
    }

    KTL_ADD_TEST(test_exotic_allocator_7)
    {
        struct debug_info
        {
            std::string_view File;
            uint_least32_t Line;
            size_t Size;
        };

        std::vector<debug_info> allocations;

        // Create the allocator with std::cerr
        debug<linear_allocator<1024>, std::vector<debug_info>> alloc(allocations);

        constexpr bool isnoex = detail::has_nothrow_allocate_v<debug<linear_allocator<1024>, std::vector<debug_info>>>;

        // Allocate and deallocate 1 double
        void* p = alloc.allocate(16);
        void* p2 = alloc.allocate(64);
        void* p3 = alloc.allocate(1024);
            
        // When it deallocates it should give a message in the standard error stream
        alloc.deallocate(p, 16);
        alloc.deallocate(p2, 64);
        alloc.deallocate(p3, 1024);
    }
}