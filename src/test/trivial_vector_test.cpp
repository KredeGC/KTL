#include "test.h"

#include "shared/assert_utility.h"

#include "ktl/containers/trivial_vector.h"

#include "ktl/allocators/freelist_allocator.h"
#include "ktl/allocators/stack_allocator.h"

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

    KTL_ADD_TEST(test_trivial_vector_freelist)
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

        trivial_vector<double, freelist_allocator<double>> vec;
        test_trivial_vector_push_back(vec, values, 8);
    }
}