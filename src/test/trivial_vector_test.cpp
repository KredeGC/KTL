#include "shared/assert_utility.h"
#include "shared/test.h"
#include "shared/types.h"

#include "ktl/ktl_fwd.h"

#include "ktl/containers/trivial_vector.h"

#include "ktl/allocators/freelist_allocator.h"
#include "ktl/allocators/stack_allocator.h"
#include "ktl/allocators/mallocator.h"

#include <vector>

namespace ktl
{
    template<typename Vec, typename T>
    void test_trivial_vector_push_back(Vec& vec, const T* values, size_t amount)
    {
        for (size_t i = 0; i < amount; i++)
            vec.push_back(values[i]);

        for (size_t i = 0; i < amount; i++)
            KTL_ASSERT(vec[i] == values[i]);
    }

    KTL_ADD_TEST(test_trivial_vector_double)
    {
        constexpr double values[] = {
            0.0,
            28.0,
            58.0,
            32.0,
            10.0,
            9.0,
            20.0,
            8.0
        };

        // TODO: Use trivial_vector instead, and add std::vector to other allocator tests
        freelist<4096> block;
        size_t* ptr = reinterpret_cast<size_t*>(block.Data + 16);
        std::vector<double, type_freelist_allocator<double, 4096>> vec({ block });
        test_trivial_vector_push_back(vec, values, 8);
    }

    KTL_ADD_TEST(test_trivial_vector_trivial)
    {
        /*trivial_t values[] = {
            { 0.0f, 0.0f },
            { 8.0f, 7.0f },
            { 10.0f, 9.0f },
            { 9.0f, 11.0f },
            { 20.0f, 3.0f },
            { 32.0f, 8.0f },
            { 28.0f, 24.0f },
            { 58.0f, 31.0f }
        };

        stack block;
        trivial_vector<trivial_t, freelist_type_allocator<trivial_t>> vec({ block });
        test_trivial_vector_push_back(vec, values, 8);*/
    }
}