#include "shared/array_utility.h"
#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/trivial_buffer.h"

// Naming scheme: test_trivial_buffer_[Alloc]_[Type]
// Contains tests that relate directly to the ktl::trivial_buffer

namespace ktl::test::trivial_buffer
{
    KTL_ADD_TEST(test_trivial_buffer_construct)
    {
        using Container = ktl::trivial_buffer<double, 4>;

        constexpr size_t size = 4;

        double values[] = {
            4.0,
            8.0,
            -1.0,
            10.0
        };

        Container baseContainer;

        assert_construct_container<Container>(
            [&](Container& lhs, Container& rhs)
        {
            // Comparison function
            for (size_t i = 0; i < size; i++)
                KTL_TEST_ASSERT(lhs[i] == rhs[i]);
        }, [&]()
        {
            // Push some elements
            for (size_t i = 0; i < size; i++)
                baseContainer[i] = values[i];

            return baseContainer;
        }, [&]()
        {
            // Construct using initializer list
            return Container{ values[0], values[1], values[2], values[3] };
        }, [&]()
        {
            // Construct from pointer range
            return Container(values, values + size);
        });
    }

    KTL_ADD_TEST(test_trivial_buffer_double)
    {
        ktl::trivial_buffer<double, 8> arr;

        
    }

    KTL_ADD_TEST(test_trivial_linear_trivial)
    {
        ktl::trivial_buffer<trivial_t, 8> arr;

        
    }
}