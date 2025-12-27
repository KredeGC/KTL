#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/inline_vector.h"

// Naming scheme: test_inline_vector_[Alloc]_[Type]
// Contains tests that relate directly to the ktl::inline_vector

namespace ktl::test::inline_vector
{
    KTL_ADD_TEST(test_inline_vector_construct)
    {
        constexpr size_t size = 4;

        using Container = ktl::inline_vector<double, size>;

        double values[]
        {
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
            KTL_TEST_ASSERT(lhs == rhs);
        }, [&]()
        {
            // Push some elements
            for (size_t i = 0; i < size; i++)
                baseContainer.push_back(values[i]);

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

    KTL_ADD_TEST(test_inline_vector_double)
    {
        ktl::inline_vector<double, 8> vec;

        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_inline_vector_trivial)
    {
        ktl::inline_vector<trivial_t, 8> vec;

        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_inline_vector_complex)
    {
        ktl::inline_vector<complex_t, 8> vec;

        assert_vector_values<complex_t>(vec);
    }
}