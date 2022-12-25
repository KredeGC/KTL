#include "shared/assert_utility.h"
#include "shared/construct_utility.h"
#include "shared/test.h"
#include "shared/types.h"
#include "shared/unordered_multimap_utility.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/unordered_multimap.h"

#include "ktl/allocators/list_allocator.h"
#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_unordered_multimap_[Alloc]_[Type]
// Contains tests that use the ktl::unordered_multimap container

namespace ktl::test::unordered_multimap
{
    KTL_ADD_TEST(test_unordered_multimap_constructors)
    {
        using Map = ktl::unordered_multimap<std::string, double>;

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
                KTL_TEST_ASSERT(lhs.find(keys[i])->second == rhs.find(keys[i])->second);
        });
    }

    KTL_ADD_TEST(test_unordered_multimap_collide_tombstone)
    {
        // Construct a map with a known size
        ktl::unordered_multimap<size_t, double> map(3); // This will result in a capacity of 8
        map.reserve(3); // This will do the same, but is included for compilation's sake

        double values[] = {
            42.0,
            58.55,
            -1.1
        };

        // All of these keys hash to 7 - h(x) % 8 == 7
        map.insert(2, values[0]);
        map.insert(10, values[1]);
        map.insert(2, values[2]);

        // Since they all collide, we expect 2 at slot 7, 10 at slot 0 and 2 at slot 1
        auto iter = map.find(2);

        KTL_TEST_ASSERT(iter->second == values[0]);
        KTL_TEST_ASSERT(map.find(10)->second == values[1]);
        KTL_TEST_ASSERT((++iter)->second == values[2]);

        // Erasing 2 places a tombstone, which cannot be reused, unless using insert()
        iter = map.find(2);
        map.erase(iter);

        KTL_TEST_ASSERT(map.find(2)); // There should still be one other element
        KTL_TEST_ASSERT(map.find(10)->second == values[1]);
        KTL_TEST_ASSERT(map.find(2)->second == values[2]);

        // Insert the old element again, overriding the tombstone
        map.insert(2, values[0]);

        // If it overrides it, the iterators should point to the same object
        KTL_TEST_ASSERT(map.find(2) == iter);

        KTL_TEST_ASSERT(iter->second == values[0]);
        KTL_TEST_ASSERT(map.find(10)->second == values[1]);
        KTL_TEST_ASSERT((++iter)->second == values[2]);
    }

    KTL_ADD_TEST(test_unordered_multimap_triple_collide)
    {
        ktl::unordered_multimap<std::string, double> map;

        // Construct a map where .png, .wav and .hdr all collide
        map.insert(".gltf", 0.0);
        map.insert(".glb", 0.0);

        map.insert(".material", 0.0);

        map.insert(".gltf", 0.0);
        map.insert(".glb", 0.0);

        map.insert(".shader", 0.0);

        map.insert(".wav", 0.0);
        map.insert(".mp3", 0.0);

        map.insert(".png", 0.0);
        map.insert(".jpeg", 0.0);
        map.insert(".jpg", 0.0);
        map.insert(".hdr", 0.0);

        size_t counter = 0;
        auto iter = map.find(".hdr");
        while (iter)
        {
            ++counter;
            ++iter;
        }

        KTL_TEST_ASSERT(counter == 1);
    }

#pragma region std::allocator
    KTL_ADD_TEST(test_unordered_multimap_std_double)
    {
        ktl::unordered_multimap<std::string, double> map;
        assert_unordered_multimap_values<double>(map);
    }

    KTL_ADD_TEST(test_unordered_multimap_std_trivial)
    {
        ktl::unordered_multimap<std::string, trivial_t> map;
        assert_unordered_multimap_values<trivial_t>(map);
    }

    KTL_ADD_TEST(test_unordered_multimap_std_packed)
    {
        ktl::unordered_multimap<std::string, packed_t> map;
        assert_unordered_multimap_values<packed_t>(map);
    }

    KTL_ADD_TEST(test_unordered_multimap_std_complex)
    {
        ktl::unordered_multimap<std::string, complex_t> map;
        assert_unordered_multimap_values<complex_t>(map);
    }
#pragma endregion
}