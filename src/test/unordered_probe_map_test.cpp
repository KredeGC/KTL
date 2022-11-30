#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/unordered_map_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/unordered_probe_map.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_unordered_probe_map_[Alloc]_[Type]
// Contains tests that use the ktl::unordered_probe_map container

namespace ktl::test::unordered_probe_map
{
    KTL_ADD_TEST(test_unordered_probe_map_constructors)
    {
        using Map = ktl::unordered_probe_map<std::string, double>;

        constexpr size_t size = 4;

        std::string keys[] = {
            "Test1",
            "Something",
            "A very long string to ensure no small-string optimization",
            "A bit longer string"
        };

        double values[] = {
            4.0,
            8.0,
            -1.0,
            10.0
        };

        assert_construct_container_base<Map>(
            [&](Map& map)
        {
            for (size_t i = 0; i < size; i++)
                map.insert(keys[i], values[i]);
        },
            [&](Map& lhs, Map& rhs)
        {
            for (size_t i = 0; i < size; i++)
                KTL_TEST_ASSERT(lhs[keys[i]] == rhs[keys[i]]);
        });
    }

#pragma region std::allocator
    KTL_ADD_TEST(test_unordered_probe_map_std_double)
    {
        ktl::unordered_probe_map<std::string, double> map;
        assert_unordered_map_values<double>(map);
    }

    KTL_ADD_TEST(test_unordered_probe_map_std_trivial)
    {
        ktl::unordered_probe_map<std::string, trivial_t> map;
        assert_unordered_map_values<trivial_t>(map);
    }

    KTL_ADD_TEST(test_unordered_probe_map_std_packed)
    {
        ktl::unordered_probe_map<std::string, packed_t> map;
        assert_unordered_map_values<packed_t>(map);
    }

    KTL_ADD_TEST(test_unordered_probe_map_std_complex)
    {
        ktl::unordered_probe_map<std::string, complex_t> map;
        assert_unordered_map_values<complex_t>(map);
    }
#pragma endregion
}