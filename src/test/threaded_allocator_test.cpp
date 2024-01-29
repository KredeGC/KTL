#include "shared/allocation_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/allocators/freelist.h"
#include "ktl/allocators/global.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/shared.h"
#include "ktl/allocators/threaded.h"
#include "ktl/allocators/type_allocator.h"

#include <vector>
#include <thread>

#if defined(__cpp_lib_latch) && __cpp_lib_latch >= 201907L
#include <latch>
#endif // __cpp_lib_latch

// Naming scheme: test_threaded_allocator_[Type]
// Contains tests that relate directly to the ktl::threaded

namespace ktl::test::threaded_allocator
{
#if defined(__cpp_lib_latch) && __cpp_lib_latch >= 201907L
    KTL_ADD_TEST(test_shared_threaded_allocator_lock)
    {
        std::latch latch(3);
        std::latch done(3);

        ktl::atomic_shared<ktl::threaded<ktl::freelist<0, 64, ktl::mallocator>>> alloc;

        auto lambda = [&]
        {
            latch.arrive_and_wait();

            auto alloc1 = alloc; // Ref-copy the allocator

            for (int i = 0; i < 1000; i++)
                assert_raw_allocate_deallocate<2, 4, 8, 16, 32, 64>(alloc1);

            done.count_down();
        };

        std::jthread thread1(lambda);
        std::jthread thread2(lambda);

        lambda();

        done.wait();
    }

    KTL_ADD_TEST(test_global_threaded_allocator_lock)
    {
        std::latch latch(3);
        std::latch done(3);

        auto lambda = [&]
        {
            latch.arrive_and_wait();

            ktl::global<ktl::threaded<ktl::freelist<0, 64, ktl::mallocator>>> alloc;

            for (int i = 0; i < 1000; i++)
                assert_raw_allocate_deallocate<2, 4, 8, 16, 32, 64>(alloc);

            done.count_down();
        };

        std::jthread thread1(lambda);
        std::jthread thread2(lambda);

        lambda();

        done.wait();
    }
#endif // __cpp_lib_latch
}