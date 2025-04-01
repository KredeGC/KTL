#include "shared/assert_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/packed_ptr.h"

#include <memory>

// Naming scheme: packed_ptr
// Contains tests that use the ktl::packed_ptr

namespace ktl::test::packed_ptr
{
    struct alignas(16) struct16
    {
        double a;
        double b;
    };

    KTL_ADD_TEST(test_packed_ptr_stack)
    {
        int t;
        int* in_ptr = &t;
        uint16_t in_value = 2;

        ktl::packed_ptr<int*, uint16_t, 2> pack(in_ptr, in_value);

        KTL_TEST_ASSERT(in_ptr == pack.get_ptr());
        KTL_TEST_ASSERT(in_value == pack.get_int());
    }

    KTL_ADD_TEST(test_packed_ptr_malloc)
    {
        std::allocator<double> alloc;

        double* in_ptr = alloc.allocate(1);
        int in_value = -2;

        // Using more than 2 bits may require full x64 support
        ktl::packed_ptr<double*, int, 2> pack;

        pack.set_ptr(in_ptr);
        pack.set_int(in_value);

        KTL_TEST_ASSERT(in_ptr == pack.get_ptr());
        KTL_TEST_ASSERT(in_value == pack.get_int());

        alloc.deallocate(in_ptr, 1);
    }

    KTL_ADD_TEST(test_packed_ptr_align_16)
    {
        struct16 ptr_value;
        struct16* in_ptr = &ptr_value;
        int in_value = -7;

        ktl::packed_ptr<struct16*, int, 4> pack;

        pack.set_ptr(in_ptr);
        pack.set_int(in_value);

        KTL_TEST_ASSERT(in_ptr == pack.get_ptr());
        KTL_TEST_ASSERT(in_value == pack.get_int());
    }
}