#include "shared/assert_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#define KTL_DEBUG_ASSERT
#include "ktl/qol/wrapper.h"

#include <vector>

// Naming scheme: packed_ptr
// Contains tests that use the ktl::in

namespace ktl::test::wrapper
{
    void test_func_in(ktl::in<int> value)
    {
        int a = 4;
        int b = a + value;
    }

    void test_func_in(ktl::in<packed_t> value)
    {
        int* p = value.ptr1;
    }

    void test_func_out(ktl::out<int> value)
    {
        value.construct(4);
    }

    void test_func_inout(ktl::inout<int> value)
    {
        int a = 4 + value;

        value = a;
    }

    void test_func_copy(ktl::copy<complex_t> value)
    {
        
    }

    void test_func_move(ktl::move<complex_t> value)
    {
        complex_t new_value(value);
    }

    KTL_ADD_TEST(test_wrapper_in)
    {
        // Test in parameter for small size
        {
            int value = 54;
            test_func_in(value);
        }

        // Test in parameter for large size
        {
            packed_t value;
            test_func_in(value);
        }

        // Test out parameter
        {
            int value;
            test_func_out(value);
        }

        // Test inout parameter
        {
            int value = 54;
            test_func_inout(value);
        }

        // Test copy parameter
        {
            complex_t value = 54.0;
            test_func_copy(value);
        }

        // Test move parameter
        {
            complex_t value = 54.0;
            test_func_move(value);
        }
    }

    KTL_ADD_TEST(test_iterator_type)
    {
        std::vector<trivial_t> vec;
        vec.emplace_back(2, 42);
        vec.emplace_back(4, 69);

        ktl::iterator iter = vec;

        trivial_t& value1 = *iter;
        double value2 = iter->gCost;
        bool has_next = iter.has_next();

        while (iter)
        {
            auto& v = iter.next();
        }

        int count = 0;
        for (auto& v : ktl::iterator(vec.begin(), vec.end()))
        {
            count++;
        }
    }
}