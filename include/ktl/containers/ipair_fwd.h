#pragma once

namespace ktl
{
    /**
     * @brief Pair of an index and value from an iterator
     * @tparam K The index type
     * @tparam V The value type
    */
    template<typename K, typename V>
    class ipair_value;

    /**
     * @brief Wraps an iterator to return an index along with the value
     * @tparam K The index typ
     * @tparam I The iterator type
    */
    template<typename K, typename I>
    class ipair_iterator;

    /**
     * @brief Wraps an iterable object for use with indexed iterators
     * @tparam K The index type
     * @tparam C The wrapped container type
    */
    template<typename K, typename C>
    class ipair_iterable;

    /**
     * @brief Wraps an iterable object to return an index along with the value
     * @param container The container to wrap. Can either be referenced or moved
     * @tparam K The index type to use
     * @tparam C The container type to wrap
	 * @return An ipair_iterable which can be iterated
    */
    template<typename K, typename C>
    ipair_iterable<K, C> ipair(C&& container) noexcept(std::is_nothrow_constructible_v<ipair_iterable<K, C>, C>);

    /**
     * @brief Wraps an iterable object to return a size_t index along with the value
     * @param container The container to wrap. Can either be referenced or moved
     * @tparam C The container type to wrap
     * @return An ipair_iterable which can be iterated
    */
    template<typename C>
    ipair_iterable<size_t, C> ipair(C&& container) noexcept(std::is_nothrow_constructible_v<ipair_iterable<size_t, C>, C>);
}