#include "shared/assert_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_alloc_fwd.h"

#define KTL_DEBUG_ASSERT
#include "ktl/containers/unordered_map.h"

#include "ktl/allocators/mallocator.h"
#include "ktl/allocators/pre_allocator.h"
#include "ktl/allocators/stack_allocator.h"

// Naming scheme: test_unordered_map_[Alloc]_[Type]
// Contains tests that use the ktl::unordered_map container

namespace ktl::test
{
    KTL_ADD_TEST(test_unordered_map_constructors)
    {
        ktl::unordered_map<std::string, size_t> map;

        constexpr size_t values[] = {
            42,
            41,
            168000
        };

        map["1"] = values[0];
        map["2"] = values[1];
        map["3"] = values[2];

        // Compile copy and move constructors and assignment operators
        ktl::unordered_map<std::string, size_t> copycmap(map);

        KTL_TEST_ASSERT(copycmap["1"] == values[0]);
        KTL_TEST_ASSERT(copycmap["2"] == values[1]);
        KTL_TEST_ASSERT(copycmap["3"] == values[2]);

        ktl::unordered_map<std::string, size_t> movecmap(std::move(map));

        KTL_TEST_ASSERT(movecmap["1"] == values[0]);
        KTL_TEST_ASSERT(movecmap["2"] == values[1]);
        KTL_TEST_ASSERT(movecmap["3"] == values[2]);

        ktl::unordered_map<std::string, size_t> copyamap;
        copyamap = copycmap;

        KTL_TEST_ASSERT(copyamap["1"] == values[0]);
        KTL_TEST_ASSERT(copyamap["2"] == values[1]);
        KTL_TEST_ASSERT(copyamap["3"] == values[2]);

        ktl::unordered_map<std::string, size_t> moveamap;
        moveamap = std::move(copycmap);

        KTL_TEST_ASSERT(moveamap["1"] == values[0]);
        KTL_TEST_ASSERT(moveamap["2"] == values[1]);
        KTL_TEST_ASSERT(moveamap["3"] == values[2]);
    }

#pragma region std::allocator
    KTL_ADD_TEST(test_unordered_map_index_erase)
    {
        ktl::unordered_map<size_t, std::string> map;

        std::string value1 = "Hello world!";
        std::string value2 = "Soomethingg elsse";
        std::string value3 = "Just for good measure";

        // 0 and 12 hashes to the same at a size of 5
        map[0] = value1;
        map[11] = value2;
        map[12] = value3;

        KTL_TEST_ASSERT(map.capacity() >= 3);
        KTL_TEST_ASSERT(map.size() == 3);

        KTL_TEST_ASSERT(map[0] == value1);
        KTL_TEST_ASSERT(map[11] == value2);
        KTL_TEST_ASSERT(map[12] == value3);

        map.erase(0);

        KTL_TEST_ASSERT(map.size() == 2);

        map.insert(0, "");

        KTL_TEST_ASSERT(map[0] == "");
        KTL_TEST_ASSERT(map[11] == value2);
        KTL_TEST_ASSERT(map[12] == value3);
    }

    KTL_ADD_TEST(test_unordered_map_find_erase)
    {
        ktl::unordered_map<size_t, std::string> map;

        std::string values[] = {
            "Hello world!",
            "Soomethingg elsse",
            "Just for good measure"
        };

        map.insert(0, values[0]);
        map.insert(11, values[1]);
        map.insert(12, values[2]);

        KTL_TEST_ASSERT(map.size() == 3);

        KTL_TEST_ASSERT(map[0] == values[0]);
        KTL_TEST_ASSERT(map[11] == values[1]);
        KTL_TEST_ASSERT(map[12] == values[2]);

        auto iter = map.find(0);

        if (iter != map.end())
        {
            map.erase(iter);

            KTL_TEST_ASSERT(map.size() == 2);
            KTL_TEST_ASSERT(map.find(0) == map.end());
        }
        else
        {
            KTL_TEST_ASSERT_FALSE();
        }
    }

    KTL_ADD_TEST(test_unordered_map_iterator)
    {
        ktl::unordered_map<size_t, std::string> map;

        std::string values[] = {
            "Hello world!",
            "Soomethingg elsse",
            "Just for good measure"
        };

        map.insert(0, values[0]);
        map.insert(11, values[1]);
        map.insert(12, values[2]);

        KTL_TEST_ASSERT(map.size() == 3);

        for (auto& [key, value] : map)
        {
            bool exists = false;
            for (size_t i = 0; i < 3; i++)
            {
                if (value == values[i])
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
                KTL_TEST_ASSERT_FALSE();
        }
    }
#pragma endregion
}