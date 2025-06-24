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

    struct alignas(2) struct2
    {
        uint16_t a;
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

        ktl::packed_ptr<const double*, int, 2> pack;

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

    KTL_ADD_TEST(test_packed_ptr_align_2)
    {
        struct2 ptr_value;
        struct2* in_ptr = &ptr_value;
        bool in_value = true;

        ktl::packed_ptr<struct2*, bool, 1> pack;

        pack.set_ptr(in_ptr);
        pack.set_int(in_value);

        KTL_TEST_ASSERT(in_ptr == pack.get_ptr());
        KTL_TEST_ASSERT(in_value == pack.get_int());
    }

#ifndef KTL_EXPLICIT_POINTER
    KTL_ADD_TEST(test_packed_ptr_implicit)
    {
        struct16 ptr_value;
        struct16* in_ptr = &ptr_value;
        int in_value = -2;

        ktl::packed_ptr<struct16*, int> pack;

        static_assert(ktl::packed_ptr<struct16*, int>::FREE_BITS == 4);

        pack = in_ptr;
        pack = in_value;

        pack->a = 1.0;

        struct16& stct = *pack;

        KTL_TEST_ASSERT(pack);

        KTL_TEST_ASSERT(in_ptr == pack);
        KTL_TEST_ASSERT(in_value == pack);
    }

    KTL_ADD_TEST(test_packed_ptr_implicit_comparison)
    {
        struct16 ptr_value;

        struct16* in_ptr = &ptr_value;
        ktl::packed_ptr<struct16*, int> pack_ptr = &ptr_value;

        static_assert(ktl::packed_ptr<struct16*, int>::FREE_BITS == 4);

        pack_ptr->a = 1.0;
        in_ptr->b = -1.0;

        struct16& pack_stct = *pack_ptr;
        struct16& in_stct = *in_ptr;

        KTL_TEST_ASSERT(pack_ptr);
        KTL_TEST_ASSERT(in_ptr);

        KTL_TEST_ASSERT(in_ptr == pack_ptr);
    }
#endif // KTL_EXPLICIT_POINTER
}