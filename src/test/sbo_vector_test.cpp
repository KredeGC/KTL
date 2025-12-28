#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/vector_utility.h"

#define KTL_DEBUG_ASSERT
#include "ktl/ktl_alloc_fwd.h"

#include "ktl/containers/sbo_vector.h"
#include "ktl/containers/trivial_vector.h"

#include "ktl/allocators/linear_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/shared.h"
#include "ktl/allocators/stack_allocator.h"
#include "ktl/allocators/type_allocator.h"

#include <vector>

// Naming scheme: test_sbo_vector_[Alloc]_[Type]
// Contains tests that relate directly to the ktl::sbo_vector

namespace ktl::test::sbo_vector
{
    KTL_ADD_TEST(test_sbo_std_vector_construct)
    {
        using Container = ktl::sbo_vector<std::vector<double>>;

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
                KTL_TEST_ASSERT(lhs == rhs);

                // Comparison function
                for (size_t i = 0; i < size; i++)
                    KTL_TEST_ASSERT(lhs[i] == rhs[i]);
            },
            [&]()
            {
                // Push some elements
                for (size_t i = 0; i < size; i++)
                    baseContainer.push_back(values[i]);

                return baseContainer;
            },
            [&]()
            {
                // Construct using initializer list
                return Container{ values[0], values[1], values[2], values[3] };
            },
            [&]()
            {
                // Construct from pointer range
                return Container(values, values + size);
            });
    }

    KTL_ADD_TEST(test_sbo_trivial_vector_construct)
    {
        using Container = ktl::sbo_vector<trivial_vector<double>>;

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
                KTL_TEST_ASSERT(lhs == rhs);

                // Comparison function
                for (size_t i = 0; i < size; i++)
                    KTL_TEST_ASSERT(lhs[i] == rhs[i]);
            },
            [&]()
            {
                // Push some elements
                for (size_t i = 0; i < size; i++)
                    baseContainer.push_back(values[i]);

                return baseContainer;
            },
            [&]()
            {
                // Construct using initializer list
                return Container{ values[0], values[1], values[2], values[3] };
            },
            [&]()
            {
                // Construct from pointer range
                return Container(values, values + size);
            });
    }

    KTL_ADD_TEST(test_sbo_trivial_vector_double)
    {
        ktl::sbo_vector<trivial_vector<double>> vec;
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_sbo_trivial_vector_trivial)
    {
        ktl::sbo_vector<trivial_vector<trivial_t>> vec;
        assert_vector_values<trivial_t>(vec);
    }

    KTL_ADD_TEST(test_sbo_trivial_vector_packed)
    {
        ktl::sbo_vector<trivial_vector<packed_t>> vec;
        assert_vector_values<packed_t>(vec);
    }

    KTL_ADD_TEST(test_sbo_std_vector_double)
    {
        ktl::sbo_vector<std::vector<double>> vec;
        assert_vector_values<double>(vec);
    }

    KTL_ADD_TEST(test_sbo_std_vector_complex)
    {
        ktl::sbo_vector<std::vector<complex_t>> vec;
        assert_vector_values<complex_t>(vec);
    }
}