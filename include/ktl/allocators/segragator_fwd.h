#pragma once

#include "reference_fwd.h"
#include "shared_fwd.h"
#include "threaded_fwd.h"
#include "type_allocator_fwd.h"

#include "../utility/builder.h"

#include <cstddef>
#include <tuple>

namespace ktl
{
    // segragator
	template<size_t Threshold, typename P, typename F>
	class segragator;

    namespace detail
    {
        // Recursive helper struct for generating the segragator type
        template<bool, typename>
        struct segragator_builder;

        // When a single type remains, just return it
        template<bool R, typename Alloc>
        struct segragator_builder<R, std::tuple<Alloc>>
        {
            using type = Alloc;
        };

        // Takes in a tuple of types and returns the final segragator
        template<bool R, typename... Ts>
        struct segragator_builder<R, std::tuple<Ts...>>
        {
            static_assert(sizeof...(Ts) % 2 != 0, "The number of allocators needs to match the number of thresholds + 1");

            // The middle of the parameter pack is the root of a complete binary tree
            // The root of a complete binary tree is (2^(log2(N)) - 1) / 2
            using middle = size_invert<R,
                (pow2<sizeof...(Ts)>::Result - 1) / 2,
                sizeof...(Ts) - 1>;

            // Split the parameter pack into 2 tuples
            using split = tuple_split_indices<1, // Offset by 1, since we don't want the middle
                std::make_index_sequence<middle::first>, // Up to, but not including the middle threshold
                std::make_index_sequence<middle::second>, // Anything after the middle threshold
                std::tuple<Ts...>>;

            // This points to the root of a complete binary tree
            using threshold = typename std::tuple_element_t<middle::first, std::tuple<Ts...>>;

            // Create this struct recursively for both sides of the middle point
            using first = typename segragator_builder<R, typename split::first>::type;
            using second = typename segragator_builder<R, typename split::second>::type;

            using type = segragator<threshold::value, first, second>;
        };
    }

    /**
     * @brief A type builder for a left-leaning segragator allocator
     * @note The amount of thresholds must be 1 less than the amount of allocators
     * @tparam ...Ts Various allocators and thresholds, in descending order
    */
    template<typename ...Ts>
    using segragator_builder_min = typename detail::segragator_builder<false, std::tuple<Ts...>>::type;

    /**
     * @brief A type builder for a right-leaning segragator allocator
     * @note The amount of thresholds must be 1 less than the amount of allocators
     * @tparam ...Ts Various allocators and thresholds, in descending order
    */
    template<typename ...Ts>
    using segragator_builder_max = typename detail::segragator_builder<true, std::tuple<Ts...>>::type;

    /**
     * @brief A shorthand way of writing std::integral_constant<size_t, N>
    */
    template<size_t N>
    using threshold = std::integral_constant<size_t, N>;

    /**
	 * @brief Shorthand for a typed segragator allocator
    */
    template<typename T, size_t Threshold, typename P, typename F>
    using type_segragator_allocator = type_allocator<T, segragator<Threshold, P, F>>;

    /**
     * @brief Shorthand for a typed, weak-reference segragator allocator
    */
    template<typename T, size_t Threshold, typename P, typename F>
    using type_reference_segragator_allocator = type_allocator<T, reference<segragator<Threshold, P, F>>>;

    /**
     * @brief Shorthand for a typed, ref-counted segragator allocator
    */
    template<typename T, size_t Threshold, typename P, typename F>
    using type_shared_segragator_allocator = type_allocator<T, shared<segragator<Threshold, P, F>>>;
}