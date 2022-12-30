#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/unordered_map_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/unordered_map.h"

#include "ktl/allocators/list_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_unordered_map_[Alloc]_[Type]
// Contains tests that use the ktl::unordered_map container

namespace ktl::test::unordered_map
{
    KTL_ADD_TEST(test_unordered_map_constructors)
    {
        using Container = ktl::unordered_map<std::string, double>;

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

        assert_construct_container<Container>(
            [&](Container& lhs, Container& rhs)
        {
            for (size_t i = 0; i < size; i++)
                KTL_TEST_ASSERT(lhs[keys[i]] == rhs[keys[i]]);
        }, [&]()
        {
            Container container;
            
            for (size_t i = 0; i < size; i++)
                container.insert(keys[i], values[i]);
            
            return container;
        });
    }

    KTL_ADD_TEST(test_unordered_map_collide_tombstone)
    {
        // Construct a map with a known size
        ktl::unordered_map<size_t, double> map(3); // This will result in a capacity of 8
        map.reserve(3); // This will do the same, but is included for compilation's sake

        double values[] = {
            42.0,
            58.55,
            -1.1
        };

        map[2] = values[0]; // h(2) % 8 hashes to 7
        map[10] = values[1]; // h(10) % 8 also hashes to 7
        map[0] = values[2]; // This doesn't collide

        // Since 2 and 10 collide, we expect 2 to be at slot 7 and 10 to wrap around at slot 0
        KTL_TEST_ASSERT(map[2] == values[0]);
        KTL_TEST_ASSERT(map[10] == values[1]);
        KTL_TEST_ASSERT(map[0] == values[2]);

        // Erasing 2 places a tombstone, which cannot be reused, unless using insert()
        auto iter = map.find(2);
        map.erase(2);

        KTL_TEST_ASSERT(map.find(2) == map.end());
        KTL_TEST_ASSERT(map[10] == values[1]);
        KTL_TEST_ASSERT(map[0] == values[2]);

        // Insert the old element again, overriding the tombstone
        map.insert(2, values[0]);

        // If it overrides it, the iterators should point to the same object
        KTL_TEST_ASSERT(map.find(2) == iter);

        KTL_TEST_ASSERT(map[2] == values[0]);
        KTL_TEST_ASSERT(map[10] == values[1]);
        KTL_TEST_ASSERT(map[0] == values[2]);
    }

#pragma region std::allocator
    KTL_ADD_TEST(test_unordered_map_std_double)
    {
        ktl::unordered_map<std::string, double> map;
        assert_unordered_map_values<double>(map);
    }

    KTL_ADD_TEST(test_unordered_map_std_trivial)
    {
        ktl::unordered_map<std::string, trivial_t> map;
        assert_unordered_map_values<trivial_t>(map);
    }

    KTL_ADD_TEST(test_unordered_map_std_packed)
    {
        ktl::unordered_map<std::string, packed_t> map;
        assert_unordered_map_values<packed_t>(map);
    }

    KTL_ADD_TEST(test_unordered_map_std_complex)
    {
        ktl::unordered_map<std::string, complex_t> map;
        assert_unordered_map_values<complex_t>(map);
    }
#pragma endregion
}