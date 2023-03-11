#pragma once

#include "../utility/assert.h"
#include "binary_heap_fwd.h"

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>

namespace ktl
{
    /**
     * @brief A priority queue implemented as a binary heap
     * @tparam T The type to use. Must be move constructible and move assignable
     * @tparam Comp The comparison function. Usually std::greater<T> or std::less<T>
     * @tparam Alloc The type of allocoator to use
    */
    template<typename T, typename Comp, typename Alloc>
    class binary_heap
    {
    private:
        static_assert(std::is_move_constructible_v<T>, "T must be move constructible");
        static_assert(std::is_move_assignable_v<T>, "T must be move assignable");

        typedef std::allocator_traits<Alloc> Traits;

    public:
        typedef T* iterator;
        typedef const T* const_iterator;

        typedef std::reverse_iterator<T*> reverse_iterator;
        typedef std::reverse_iterator<const T*> const_reverse_iterator;

    public:
        /**
         * @brief Construct the binary heap with the given comparator
         * @param comp The comparator to use. Will be default constructed if unspecified
        */
        binary_heap(const Comp& comp = Comp()) noexcept :
            m_Alloc(),
            m_Comp(comp),
            m_Size(0),
            m_Capacity(0),
            m_Begin(nullptr) {}

        /**
         * @brief Construct the binary heap with the given allocator and comparator
         * @param allocator The allocator to use
         * @param comp The comparator to use. Will be default constructed if unspecified
        */
        binary_heap(const Alloc& allocator, const Comp& comp = Comp()) noexcept :
            m_Alloc(allocator),
            m_Comp(comp),
            m_Size(0),
            m_Capacity(0),
            m_Begin(nullptr) {}

        /**
         * @brief Construct the binary heap with the given comparator and an initial capacity
         * @param capacity The initial capacity to use
         * @param comp The comparator to use. Will be default constructed if unspecified
        */
        explicit binary_heap(size_t capacity, const Comp& comp = Comp()) noexcept :
            m_Alloc(),
            m_Comp(comp),
            m_Size(0),
            m_Capacity(capacity),
            m_Begin(Traits::allocate(m_Alloc, capacity)) {}

        /**
         * @brief Construct the binary heap with the given allocator, comparator and an initial capacity
         * @param capacity The initial capacity to use
         * @param allocator The allocator to use. Will be default constructed if unspecified
         * @param comp The comparator to use. Will be default constructed if unspecified
        */
        explicit binary_heap(size_t capacity, const Alloc& allocator, const Comp& comp = Comp()) noexcept :
            m_Alloc(allocator),
            m_Comp(comp),
            m_Size(0),
            m_Capacity(capacity),
            m_Begin(Traits::allocate(m_Alloc, capacity)) {}

        /**
         * @brief Construct the binary heap with the given comparator and an initial set of values
         * @param initializer A list of values to insert into the binary heap
         * @param comp The comparator to use. Will be default constructed if unspecified
        */
        binary_heap(std::initializer_list<T> initializer, const Comp& comp = Comp()) :
            m_Alloc(),
            m_Comp(comp),
            m_Size(0),
            m_Capacity(initializer.size()),
            m_Begin(Traits::allocate(m_Alloc, initializer.size()))
        {
            for (auto value : initializer)
                insert(value);
        }

        /**
         * @brief Construct the binary heap with the given allocator, comparator and an initial set of values
         * @param initializer A list of values to insert into the binary heap
         * @param allocator The allocator to use. Will be default constructed if unspecified
         * @param comp The comparator to use. Will be default constructed if unspecified
        */
        binary_heap(std::initializer_list<T> initializer, const Alloc& allocator, const Comp& comp = Comp()) :
            m_Alloc(allocator),
            m_Comp(comp),
            m_Size(0),
            m_Capacity(initializer.size()),
            m_Begin(Traits::allocate(m_Alloc, initializer.size()))
        {
            for (auto value : initializer)
                insert(value);
        }

        binary_heap(const binary_heap& other) noexcept(std::is_nothrow_copy_constructible_v<T>) :
            m_Alloc(Traits::select_on_container_copy_construction(other.m_Alloc)),
            m_Comp(other.m_Comp),
            m_Size(other.m_Size),
            m_Capacity(other.m_Size),
            m_Begin(Traits::allocate(m_Alloc, other.m_Size))
        {
            for (size_t i = 0; i < m_Size; i++)
                Traits::construct(m_Alloc, m_Begin + i, other.m_Begin[i]);
        }

        binary_heap(binary_heap&& other) noexcept(std::is_nothrow_move_constructible_v<T>) :
            m_Alloc(std::move(other.m_Alloc)),
            m_Comp(other.m_Comp),
            m_Size(other.m_Size),
            m_Capacity(other.m_Capacity),
            m_Begin(std::move(other.m_Begin))
        {
            other.m_Capacity = 0;
            other.m_Size = 0;
            other.m_Begin = nullptr;
        }

        binary_heap(const binary_heap& other, const Alloc& allocator) noexcept(std::is_nothrow_copy_constructible_v<T>) :
            m_Alloc(allocator),
            m_Comp(other.m_Comp),
            m_Size(other.m_Size),
            m_Capacity(other.m_Size),
            m_Begin(Traits::allocate(m_Alloc, other.m_Size))
        {
            for (size_t i = 0; i < m_Size; i++)
                Traits::construct(m_Alloc, m_Begin + i, other.m_Begin[i]);
        }

        binary_heap(binary_heap&& other, const Alloc& allocator) noexcept(std::is_nothrow_move_constructible_v<T>) :
            m_Alloc(allocator),
            m_Comp(std::move(other.m_Comp)),
            m_Size(other.m_Size),
            m_Capacity(other.m_Size),
            m_Begin(Traits::allocate(m_Alloc, other.m_Size))
        {
            // Moving using a different allocator means we can't just move, we have to reallocate
            for (size_t i = 0; i < m_Size; i++)
                Traits::construct(m_Alloc, m_Begin + i, other.m_Begin[i]);

            if (other.m_Begin != nullptr)
                Traits::deallocate(other.m_Alloc, other.m_Begin, other.m_Capacity * sizeof(T));

            other.m_Capacity = 0;
            other.m_Size = 0;
            other.m_Begin = nullptr;
        }

        ~binary_heap()
        {
            // Deconstruct elements
            if (m_Begin)
            {
                for (size_t i = 0; i < m_Size; i++)
                    Traits::destroy(m_Alloc, m_Begin + i);

                Traits::deallocate(m_Alloc, m_Begin, m_Capacity);
            }
        }

        binary_heap& operator=(const binary_heap& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
        {
            if (m_Begin)
            {
                for (size_t i = 0; i < m_Size; i++)
                    Traits::destroy(m_Alloc, m_Begin + i);

                Traits::deallocate(m_Alloc, m_Begin, m_Capacity);
            }

            m_Alloc = other.m_Alloc;
            m_Comp = other.m_Comp;
            m_Size = other.m_Size;
            m_Capacity = other.m_Size;
            m_Begin = Traits::allocate(m_Alloc, m_Capacity);

            // Copy construct elements
            for (size_t i = 0; i < m_Size; i++)
                Traits::construct(m_Alloc, m_Begin + i, other.m_Begin[i]);

            return *this;
        }

        binary_heap& operator=(binary_heap&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
        {
            // Deconstruct elements
            if (m_Begin)
            {
                for (size_t i = 0; i < m_Size; i++)
                    Traits::destroy(m_Alloc, m_Begin + i);

                Traits::deallocate(m_Alloc, m_Begin, m_Capacity);
            }

            m_Alloc = std::move(other.m_Alloc);
            m_Comp = std::move(other.m_Comp);
            m_Size = other.m_Size;
            m_Capacity = other.m_Capacity;
            m_Begin = other.m_Begin;

            other.m_Capacity = 0;
            other.m_Size = 0;
            other.m_Begin = nullptr;

            return *this;
        }


        iterator begin() noexcept { return m_Begin; }

        const_iterator begin() const noexcept { return m_Begin; }

        iterator end() noexcept { return m_Begin + m_Size; }

        const_iterator end() const noexcept { return m_Begin + m_Size; }

        reverse_iterator rbegin() noexcept { return std::reverse_iterator(m_Begin + m_Size); }

        const_reverse_iterator rbegin() const noexcept { return std::reverse_iterator(m_Begin + m_Size); }

        reverse_iterator rend() noexcept { return std::reverse_iterator(m_Begin); }

        const_reverse_iterator rend() const noexcept { return std::reverse_iterator(m_Begin); }

        
        /**
         * @brief Returns an iterator to the start of the heap.
         * @return An iterator to the start of the heap.
        */
        iterator data() noexcept { return m_Begin; }

        /**
         * @brief Returns a const iterator to the start of the heap.
         * @return A const iterator to the start of the heap.
        */
        const_iterator data() const noexcept { return m_Begin; }

        /**
         * @brief Returns the current size of the heap.
		 * @return The current size of the heap in number of elements.
        */
        size_t size() const noexcept { return m_Size; }

        /**
         * @brief Returns the current capacity of the heap.
         * @return The current capacity of the heap in number of elements.
        */
        size_t capacity() const noexcept { return m_Capacity; }

        /**
         * @brief Returns true if the heap has no elements.
         * @return Whether the heap has a size of 0.
        */
        bool empty() const noexcept { return m_Size == 0; }

        
		/**
		 * @brief Reserves the capacity of the heap to @p n, without initializing any elements.
		 * @param n The minimum capacity of the heap.
		*/
		void reserve(size_t n) noexcept
		{
			if (capacity() < n)
				set_size(n);
		}

        /**
         * @brief Inserts a new element into the heap via perfect forwarding.
         * @tparam V The type to insert.
         * @param value The element to insert into the heap.
         * @return An iterator to the element that was added.
        */
        template<typename V>
        iterator insert(V&& value) noexcept
        {
            if (m_Size == m_Capacity)
                expand(1);

            Traits::construct(m_Alloc, m_Begin + m_Size, std::forward<V>(value));

            size_t index = percolateUp(m_Size++);

            return m_Begin + index;
        }

        /**
         * @brief Peeks at the root element (lowest or highest, depending on min or max heap) and returns a reference to it.
         * @return A reference to the root element.
        */
        T& peek() noexcept
        {
            KTL_ASSERT(m_Size > 0);

            return m_Begin[0];
        }

        /**
         * @brief Removes the root element (lowest or highest, depending on min or max heap) and returns it.
         * @return The removed root element, returned by value.
        */
        T pop() noexcept
        {
            KTL_ASSERT(m_Size > 0);

            T root = m_Begin[0];

            m_Begin[0] = std::move(m_Begin[--m_Size]);
            Traits::destroy(m_Alloc, m_Begin + m_Size);
            percolateDown(0);

            return root;
        }

        /**
         * @brief Returns an iterator to the element @p index. Takes O(n) time.
         * @param value The value to return the iterator of.
         * @return An iterator to the given value or end() if not found.
        */
        iterator find(const T& value) const noexcept
        {
            for (size_t i = 0; i < m_Size; i++)
            {
                if (m_Begin[i] == value) // TODO: Use std::equal_to
                    return m_Begin + i;
            }

            return m_Begin + m_Size;
        }

        /**
         * @brief Clear all elements in the heap, destroying each element.
        */
        void clear() noexcept
        {
            for (size_t i = 0; i < m_Size; i++)
                Traits::destroy(m_Alloc, m_Begin + i);

            m_Size = 0;
        }

    private:
        void expand(size_t n) noexcept
        {
            size_t curCap = capacity();
            size_t alSize = curCap + (std::max)(curCap, n);

            set_size(alSize);
        }

        void set_size(size_t n) noexcept
        {
            size_t curSize = (std::min)(size(), n);

            T* newData = Traits::allocate(m_Alloc, n);

            if (m_Begin)
            {
                // Move construct elements
                for (size_t i = 0; i < curSize; i++)
                    Traits::construct(m_Alloc, newData + i, std::move(m_Begin[i]));

                // Deconstruct elements
                for (size_t i = 0; i < m_Size; i++)
                    Traits::destroy(m_Alloc, m_Begin + i);

                // Deallocate old array
                Traits::deallocate(m_Alloc, m_Begin, m_Capacity);
            }

            m_Size = curSize;
            m_Capacity = n;
            m_Begin = newData;
        }

        constexpr size_t parent(size_t index) const noexcept
        {
            return (index - 1) / 2;
        }

        constexpr size_t left(size_t index) const noexcept
        {
            return index * 2 + 1;
        }

        constexpr size_t right(size_t index) const noexcept
        {
            return index * 2 + 2;
        }

        size_t percolateUp(size_t index)
        {
            const T value = m_Begin[index];

            while (index != 0 && m_Comp(value, m_Begin[parent(index)]))
            {
                m_Begin[index] = std::move(m_Begin[parent(index)]);
                index = parent(index);
            }

            m_Begin[index] = std::move(value);

            return index;
        }

        void percolateDown(size_t index) noexcept
        {
            size_t parent = index;

            const T value = m_Begin[index];

            while (parent < m_Size)
            {
                size_t l = left(parent);
                size_t r = right(parent);

                size_t child = parent;

                if (l < m_Size && m_Comp(m_Begin[l], m_Begin[child]))
                    child = l;

                if (r < m_Size && m_Comp(m_Begin[r], m_Begin[child]))
                    child = r;

                if (child == parent)
                    break;

                const T temp(std::move(m_Begin[child]));
                m_Begin[child] = std::move(m_Begin[parent]);
                m_Begin[parent] = std::move(temp);

                parent = child;
            }

            m_Begin[parent] = std::move(value);
        }

    private:
        Alloc m_Alloc;
        Comp m_Comp;

        size_t m_Size;
        size_t m_Capacity;
        T* m_Begin;
    };
}