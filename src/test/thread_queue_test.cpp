#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#if __cpp_lib_move_only_function >= 202110L

#define KTL_DEBUG_ASSERT
#include "ktl/utility/thread_queue.h"

#include <thread>
#include <latch>

// Naming scheme: test_thread_queue_[Alloc]_[Type]
// Contains tests that relate directly to the ktl::thread_queue

namespace ktl::test::thread_queue
{
    KTL_ADD_TEST(test_thread_queue)
    {
        ktl::thread_queue queue{};

        auto main_thread_id = std::this_thread::get_id();

        std::latch latch{ 1 };

        std::jthread worker_thread([&]
        {
            latch.count_down();

            KTL_TEST_ASSERT(main_thread_id != std::this_thread::get_id());

            auto future = queue.post([main_thread_id]
            {
                KTL_TEST_ASSERT(main_thread_id == std::this_thread::get_id());
            });

            future.wait_for(std::chrono::seconds(1));
        });

        latch.wait();

        KTL_TEST_ASSERT(queue.run_once());
    }
}

#endif