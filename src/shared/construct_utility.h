#pragma once

#include "shared/assert_utility.h"

namespace ktl::test
{
    template<typename Container, typename TestFunc, typename InitFunc, typename ...Funcs>
    void assert_construct_variadic(Container& baseContainer, TestFunc test, InitFunc init, Funcs&& ...funcs)
    {
        Container container = init();
        
        test(container, baseContainer);
        
        if constexpr (sizeof...(Funcs) > 0)
            assert_construct_variadic(baseContainer, test, funcs...);
    }
    
    template<typename Container, typename TestFunc, typename InitFunc, typename ...Funcs>
    void assert_construct_container(TestFunc test, InitFunc init, Funcs&& ...funcs)
    {
        // Default constructor
        Container baseContainer = init();

        // Copy constructor
        Container copyContainer(baseContainer);

        test(copyContainer, baseContainer);

        // Move constructor
        Container moveContainer(std::move(copyContainer));

        KTL_TEST_ASSERT(copyContainer.empty());

        test(moveContainer, baseContainer);

        // Copy assignment operator
        Container copyAssignmentContainer;
        copyAssignmentContainer = baseContainer;

        test(copyAssignmentContainer, baseContainer);

        // Move assignment operator
        Container moveAssignmentContainer;
        moveAssignmentContainer = std::move(copyAssignmentContainer);

        KTL_TEST_ASSERT(copyAssignmentContainer.empty());

        test(moveAssignmentContainer, baseContainer);
        
        // Run any additional construct tests
        if constexpr (sizeof...(Funcs))
            assert_construct_variadic(baseContainer, test, funcs...);
    }
}