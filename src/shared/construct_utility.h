#pragma once

#include "shared/assert_utility.h"

namespace ktl::test
{
    template<typename Container, typename InitFunc, typename TestFunc>
    void assert_construct_container_base(InitFunc init, TestFunc func)
    {
        // Default constructor
        Container baseContainer;
        init(baseContainer);

        // Copy constructor
        Container copyContainer(baseContainer);

        func(copyContainer, baseContainer);

        // Move constructor
        Container moveContainer(std::move(copyContainer));

        KTL_TEST_ASSERT(copyContainer.empty());

        func(moveContainer, baseContainer);

        // Copy assignment operator
        Container copyAssignmentContainer;
        copyAssignmentContainer = baseContainer;

        func(copyAssignmentContainer, baseContainer);

        // Move assignment operator
        Container moveAssignmentContainer;
        moveAssignmentContainer = std::move(copyAssignmentContainer);

        KTL_TEST_ASSERT(copyAssignmentContainer.empty());

        func(moveAssignmentContainer, baseContainer);
    }


    template<typename Container>
	void assert_construct_container()
	{
        constexpr size_t size = 8;

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

        // Size constructor
        Container baseArray(size);

        for (size_t i = 0; i < size; i++)
            baseArray[i] = values[i];

        // Copy constructor
        Container copyArray(baseArray);

        for (size_t i = 0; i < size; i++)
            KTL_TEST_ASSERT(values[i] == copyArray[i]);

        // Move constructor
        Container moveArray(std::move(copyArray));

        KTL_TEST_ASSERT(copyArray.empty());

        for (size_t i = 0; i < size; i++)
            KTL_TEST_ASSERT(values[i] == moveArray[i]);

        // initializer_list constructor
        Container initArray({ values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7] });

        for (size_t i = 0; i < size; i++)
            KTL_TEST_ASSERT(values[i] == initArray[i]);

        // Pointer constructor
        Container pointArray(values, values + size);

        for (size_t i = 0; i < size; i++)
            KTL_TEST_ASSERT(values[i] == pointArray[i]);

        // Copy assignment operator
        Container copyAssignmentArray;
        copyAssignmentArray = baseArray;

        for (size_t i = 0; i < size; i++)
            KTL_TEST_ASSERT(values[i] == copyAssignmentArray[i]);

        // Move assignment operator
        Container moveAssignmentArray;
        moveAssignmentArray = std::move(copyAssignmentArray);

        for (size_t i = 0; i < size; i++)
            KTL_TEST_ASSERT(values[i] == moveAssignmentArray[i]);
	}
}