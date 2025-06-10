#pragma once

#include "../utility/empty_base.h"

#include "ipair_fwd.h"

#include <cstddef>
#include <iterator>
#include <optional>

namespace ktl
{
    namespace detail
    {
        // has begin() && end()
        template<typename T, typename = void>
        struct is_iterable : std::false_type {};

        template<typename T>
        struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> : std::true_type {};

        template<typename T>
        constexpr bool is_iterable_v = is_iterable<T>::value;

        // has operator--()
        template<typename T, typename = void>
        struct is_decrementable : std::false_type {};

        template<typename T>
        struct is_decrementable<T, std::void_t<decltype(--std::declval<T&>())>> : std::true_type {};

        template<typename T>
        constexpr bool is_decrementable_v = is_decrementable<T>::value;
    }

    template<typename K, typename V>
    class ipair_value
    {
    public:
        template<typename A, typename B>
        ipair_value(A&& key, B&& value) noexcept(std::is_nothrow_constructible_v<K, A> && std::is_nothrow_constructible_v<V, B>) :
            first(std::forward<A>(key)),
            second(std::forward<B>(value)) {}

        ipair_value* operator->() noexcept
        {
            return this;
        }

        const ipair_value* operator->() const noexcept
        {
            return this;
        }

        KTL_EMPTY_BASE K first;
        KTL_EMPTY_BASE V second;
    };

    template<typename K, typename I>
    class ipair_iterator
    {
    private:
        using V = decltype(*std::declval<I&>());

    public:
        //using iterator_category = typename std::iterator_traits<I>::iterator_category;
        //using difference_type = typename std::iterator_traits<I>::difference_type;
        using value_type = ipair_value<K, V>;
        using pointer = value_type*;
        using reference = value_type&;

    public:
        ipair_iterator(I&& iterator) noexcept(std::is_nothrow_default_constructible_v<K> && std::is_nothrow_move_constructible_v<I>) :
            m_Iterator(std::move(iterator)),
            m_Index{} {}

        value_type operator*() const noexcept(std::is_nothrow_constructible_v<value_type, K, I> && noexcept(*std::declval<I&>()))
        {
            return value_type(m_Index, *m_Iterator);
        }

        value_type operator->() const noexcept(std::is_nothrow_constructible_v<value_type, K, I> && noexcept(*std::declval<I&>()))
        {
            return value_type(m_Index, *m_Iterator);
        }

        ipair_iterator& operator++() noexcept(noexcept(++std::declval<K&>()) && noexcept(++std::declval<I&>()))
        {
            ++m_Iterator;
            ++m_Index;
            return *this;
        }

        template<typename IT = I>
        typename std::enable_if_t<std::is_copy_constructible_v<ipair_iterator<K, IT>>, ipair_iterator>
        operator++(int) noexcept(std::is_nothrow_copy_constructible_v<ipair_iterator> && noexcept(++std::declval<K&>()) && noexcept(++std::declval<I&>()))
        {
            ipair_iterator iter = *this;
            ++m_Iterator;
            ++m_Index;
            return iter;
        }

        template<typename IT = I>
        typename std::enable_if_t<detail::is_decrementable_v<IT>, ipair_iterator&>
        operator--() noexcept(noexcept(--std::declval<K&>()) && noexcept(--std::declval<I&>()))
        {
            --m_Iterator;
            --m_Index;
            return *this;
        }

        template<typename IT = I>
        typename std::enable_if_t<std::is_copy_constructible_v<ipair_iterator<K, IT>> && detail::is_decrementable_v<IT>, ipair_iterator>
        operator--(int) noexcept(std::is_nothrow_copy_constructible_v<ipair_iterator> && noexcept(--std::declval<K&>()) && noexcept(--std::declval<I&>()))
        {
            ipair_iterator iter = *this;
            --m_Iterator;
            --m_Index;
            return iter;
        }

        bool operator==(const ipair_iterator& b) const noexcept(noexcept(std::declval<const I&>() == std::declval<const I&>()))
        {
            return m_Iterator == b.m_Iterator;
        }

        bool operator!=(const ipair_iterator& b) const noexcept(noexcept(std::declval<const I&>() != std::declval<const I&>()))
        {
            return m_Iterator != b.m_Iterator;
        }

    private:
        KTL_EMPTY_BASE I m_Iterator;
        KTL_EMPTY_BASE K m_Index;
    };

    template<typename K, typename C>
    class ipair_iterable
    {
    private:
        static_assert(detail::is_iterable_v<C>, "Type must be iterable");

        using begin_type = decltype(std::declval<C&>().begin());
        using begin_const_type = decltype(std::declval<const C&>().begin());

    public:
        using iterator = ipair_iterator<K, begin_type>;
        using const_iterator = ipair_iterator<K, begin_const_type>;

    public:
        template<typename V>
        ipair_iterable(V&& container) noexcept(std::is_nothrow_constructible_v<C, V>) :
            m_Container(std::forward<V>(container)) {}

        template<typename V = C>
        typename std::enable_if_t<detail::is_iterable_v<std::decay_t<V>>, iterator>
        begin() noexcept(noexcept(std::declval<V&>().begin()) && noexcept(iterator(std::declval<begin_type>())))
        {
            return iterator(m_Container.begin());
        }

        template<typename V = C>
        typename std::enable_if_t<detail::is_iterable_v<std::decay_t<V>>, iterator>
        end() noexcept(noexcept(std::declval<V&>().end()) && noexcept(iterator(std::declval<begin_type>())))
        {
            return iterator(m_Container.end());
        }

        template<typename V = C>
        typename std::enable_if_t<detail::is_iterable_v<std::add_const_t<std::decay_t<V>>>, const_iterator>
        begin() const noexcept(noexcept(std::declval<const V&>().begin()) && noexcept(const_iterator(std::declval<begin_type>())))
        {
            return const_iterator(m_Container.begin());
        }

        template<typename V = C>
        typename std::enable_if_t<detail::is_iterable_v<std::add_const_t<std::decay_t<V>>>, const_iterator>
        end() const noexcept(noexcept(std::declval<const V&>().end()) && noexcept(const_iterator(std::declval<begin_type>())))
        {
            return const_iterator(m_Container.end());
        }

    private:
        KTL_EMPTY_BASE C m_Container;
    };

    template<typename K, typename C>
    ipair_iterable<K, C> ipair(C&& container) noexcept(std::is_nothrow_constructible_v<ipair_iterable<K, C>, C>)
    {
        return ipair_iterable<K, C>(std::forward<C>(container));
    }

    template<typename C>
    ipair_iterable<size_t, C> ipair(C&& container) noexcept(std::is_nothrow_constructible_v<ipair_iterable<size_t, C>, C>)
    {
        return ipair<size_t, C>(std::forward<C>(container));
    }
}