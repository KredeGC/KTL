#pragma once

#include "shared_fwd.h"
#include "threaded_fwd.h"
#include "type_allocator_fwd.h"

#include "../utility/builder.h"

#include <cstddef>
#include <tuple>

namespace ktl
{
    // type_allocator
	template<typename T, typename Alloc>
	class type_allocator;
    
	// fallback
	template<typename P, typename F>
	class fallback;

    namespace detail
    {
        // Recursive helper struct for generating the segragator type
        template<bool, typename>
        struct fallback_builder;

        // When a single type remains, just return it
        template<bool R, typename Alloc>
        struct fallback_builder<R, std::tuple<Alloc>>
        {
            using type = Alloc;
        };

        // Takes in a tuple of types and returns the final fallback
        template<bool R, typename... Ts>
        struct fallback_builder<R, std::tuple<Ts...>>
        {
            // The middle of the parameter pack is the root of a complete binary tree
            // The pack can be split in 2 using 2^(log2(N)) / 2
            using middle = size_invert<R,
                pow2<sizeof...(Ts)>::Result / 2,
                sizeof...(Ts)>;

            // Split the parameter pack into 2 tuples
            using split = tuple_split_indices<0, // No offset
                std::make_index_sequence<middle::first>, // One half, up to the middle point
                std::make_index_sequence<middle::second>, // Anything after the middle threshold
                std::tuple<Ts...>>;

            // Create this struct recursively for both sides of the middle point
            using first = typename fallback_builder<R, typename split::first>::type;
            using second = typename fallback_builder<R, typename split::second>::type;

            using type = fallback<first, second>;
        };
    }
    
    /**
     * @brief A type builder for a left-leaning fallback allocator
     * @tparam ...Ts Various allocators, in descending order
    */
    template<typename... Ts>
    using fallback_builder_min = typename detail::fallback_builder<false, std::tuple<Ts...>>::type;

    /**
     * @brief A type builder for a right-leaning fallback allocator
     * @tparam ...Ts Various allocators, in descending order
    */
    template<typename... Ts>
    using fallback_builder_max = typename detail::fallback_builder<true, std::tuple<Ts...>>::type;

    /**
     * @brief Shorthand for a typed fallback allocator
    */
    template<typename T, typename P, typename F>
    using type_fallback_allocator = type_allocator<T, fallback<P, F>>;

    /**
     * @brief Shorthand for a typed, ref-counted fallback allocator
    */
    template<typename T, typename P, typename F>
    using type_shared_fallback_allocator = type_allocator<T, shared<fallback<P, F>>>;

    /**
     * @brief Shorthand for a typed, thread-safe, ref-counted fallback allocator
    */
    template<typename T, typename P, typename F>
    using type_threaded_fallback_allocator = type_allocator<T, threaded<fallback<P, F>>>;
}