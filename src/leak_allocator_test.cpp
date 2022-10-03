#include "leak_allocator_test.h"
#include "allocation_utility.h"

#include "ktl/binary_heap.h"
#include "ktl/composite_allocator.h"
#include "ktl/leak_allocator.h"
#include "ktl/freelist_allocator.h"
#include "ktl/stack_allocator.h"

#include <sstream>

namespace ktl
{
    static std::stringbuf stringBuffer;
    static std::ostream stringOut(&stringBuffer);

    void test_freelist_leak()
    {
        stringBuffer = std::stringbuf();

        {
            constexpr double value1 = 42.5;
            constexpr double value2 = 81.3;
            constexpr double value3 = 384.6;
            constexpr double value4 = 182.1;
            constexpr double value5 = 99.9;

            leak_allocator<freelist_allocator<double>, stringOut> alloc;

            double* ptr1 = assert_allocate<double>(alloc, value1);
            double* ptr2 = assert_allocate<double>(alloc, value2);
            double* ptr3 = assert_allocate<double>(alloc, value3);

            KTL_ASSERT(*ptr1 == value1);
            KTL_ASSERT(*ptr2 == value2);
            KTL_ASSERT(*ptr3 == value3);

            assert_deallocate<double>(alloc, ptr1);

            double* ptr4 = assert_allocate<double>(alloc, value4);
            double* ptr5 = assert_allocate<double>(alloc, value5);

            KTL_ASSERT(*ptr4 == value4);
            KTL_ASSERT(*ptr5 == value5);

            assert_deallocate<double>(alloc, ptr4);
            assert_deallocate<double>(alloc, ptr2);
            assert_deallocate<double>(alloc, ptr3);
            assert_deallocate<double>(alloc, ptr5);
        }

        std::cout << stringBuffer.str();

        KTL_ASSERT(stringBuffer.str().empty());
    }

    void test_binary_heap_leak()
    {
        stringBuffer = std::stringbuf();

        {
            constexpr double values[] = {
                0.0,
                8.0,
                9.0,
                10.0,
                20.0,
                28.0,
                32.0,
                58.0
            };

            binary_min_heap<double, leak_allocator<freelist_allocator<double>, stringOut>> heap(3);
            heap.insert(values[0]);
            heap.insert(values[5]);
            heap.insert(values[7]);
            heap.insert(values[6]);
            heap.insert(values[3]);
            heap.insert(values[2]);
            heap.insert(values[4]);
            heap.insert(values[1]);

            size_t size = heap.size();
            for (size_t i = 0; i < size; i++)
                KTL_ASSERT(heap.pop() == values[i]);

            /*auto data = heap.data();
            for (size_t i = 0; i < size; i++)
                std::cout << data[i] << std::endl;

            for (size_t i = 0; i < size; i++)
                std::cout << heap.pop() << std::endl;*/
        }

        std::cout << stringBuffer.str();

        KTL_ASSERT(stringBuffer.str().empty());
    }
}