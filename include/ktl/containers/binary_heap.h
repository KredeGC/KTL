#pragma once

#include "binary_heap_fwd.h"

#include <cstddef>
#include <iterator>
#include <memory>
#include <utility>

namespace ktl
{
    template<typename T, typename Comp, typename Alloc>
    class binary_heap
    {
    private:
        typedef std::allocator_traits<Alloc> Traits;

    public:
        typedef T* iterator;
        typedef const T* const_iterator;

        typedef std::reverse_iterator<T*> reverse_iterator;
        typedef std::reverse_iterator<const T*> const_reverse_iterator;

    public:
        binary_heap(const Alloc& allocator = Alloc(), const Comp& comp = Comp()) noexcept :
            m_Alloc(allocator),
            m_Comp(comp),
            m_Size(0),
            m_Capacity(0),
            m_Begin(nullptr) {}

        explicit binary_heap(size_t capacity, const Alloc& allocator = Alloc(), const Comp& comp = Comp()) noexcept :
            m_Alloc(allocator),
            m_Comp(comp),
            m_Size(0),
            m_Capacity(capacity),
            m_Begin(Traits::allocate(m_Alloc, capacity)) {}

        binary_heap(const binary_heap& other) noexcept(std::is_nothrow_copy_constructible_v<T>) :
            m_Alloc(Traits::select_on_container_copy_construction(static_cast<Alloc>(other))),
            m_Comp(other.m_Comp),
            m_Size(other.m_Size),
            m_Capacity(other.m_Size),
            m_Begin(Traits::allocate(m_Alloc, other.m_Size))
        {
            for (size_t i = 0; i < m_Size; i++)
                Traits::construct(m_Alloc, m_Begin + i, other.m_Begin[i]);
        }

        binary_heap(binary_heap&& other) noexcept(std::is_nothrow_move_constructible_v<T>) :
            m_Alloc(std::move(other)),
            m_Comp(other.m_Comp),
            m_Size(other.m_Size),
            m_Capacity(other.m_Capacity),
            m_Begin(std::move(other.m_Begin))
        {
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
         * @return Returns an iterator to the start of the heap.
        */
        iterator data() noexcept { return m_Begin; }

        /**
         * @brief Returns a const iterator to the start of the heap.
         * @return Returns a const iterator to the start of the heap.
        */
        const_iterator data() const noexcept { return m_Begin; }

        /**
         * @brief Returns the current size of the heap.
         * @return Returns the current size of the heap.
        */
        size_t size() const noexcept { return m_Size; }

        /**
         * @brief Returns the current capacity of the heap.
         * @return Returns the current capacity of the heap.
        */
        size_t capacity() const noexcept { return m_Capacity; }

        /**
         * @brief Returns true if the heap has no elements.
         * @return Returns true if the heap has no elements.
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
         * @brief Pushes a new element into the heap by copying it.
         * @param value The element to copy into the heap.
         * @return An iterator to the element that was added.
        */
        iterator insert(const T& value) noexcept
        {
            if (m_Size == m_Capacity)
                expand(1);

            const size_t lastElement = m_Size;
            size_t hole = m_Size++;
            while (hole != 0 && m_Comp(value, m_Begin[parent(hole)]))
            {
                if (hole == lastElement)
                    Traits::construct(m_Alloc, m_Begin + hole, std::move(m_Begin[parent(hole)]));
                else
                    m_Begin[hole] = std::move(m_Begin[parent(hole)]);
                hole = parent(hole);
            }

            if (hole == lastElement)
                Traits::construct(m_Alloc, m_Begin + hole, value);
            else
                m_Begin[hole] = value;

            return m_Begin + hole;
        }

        /**
         * @brief Pushes a new element into the heap by moving it.
         * @param value The element to move into the heap.
         * @return An iterator to the element that was added.
        */
        iterator insert(T&& value) noexcept
        {
            if (m_Size == m_Capacity)
                expand(1);

            const size_t lastElement = m_Size;
            size_t hole = m_Size++;
            while (hole != 0 && m_Comp(value, m_Begin[parent(hole)]))
            {
                if (hole == lastElement)
                    Traits::construct(m_Alloc, m_Begin + hole, std::move(m_Begin[parent(hole)]));
                else
                    m_Begin[hole] = std::move(m_Begin[parent(hole)]);
                hole = parent(hole);
            }

            if (hole == lastElement)
                Traits::construct(m_Alloc, m_Begin + hole, std::move(value));
            else
                m_Begin[hole] = std::move(value);

            return m_Begin + hole;
        }

        /**
         * @brief Peeks at the root element (lowest or highest, depending on min or max heap) and returns a reference to it.
         * @return A reference to the root element.
        */
        T& peek() noexcept
        {
            return m_Begin[0];
        }

        /**
         * @brief Removes the root element (lowest or highest, depending on min or max heap) and returns it.
         * @return The removed root element, returned by value.
        */
        T pop() noexcept
        {
            T root = m_Begin[0];

            if (m_Size > 0)
            {
                m_Begin[0] = std::move(m_Begin[--m_Size]);
                Traits::destroy(m_Alloc, m_Begin + m_Size);
                heapify(0);
            }

            return root;
        }

        /**
         * @brief Returns an iterator to the element `index`. Takes O(n) time.
         * @param value The value to return the iterator of.
         * @return An iterator to the given value or `end()` if not found.
        */
        iterator find(const T& value) const noexcept
        {
            for (size_t i = 0; i < m_Size i++)
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

        void heapify(size_t index) noexcept
        {
            size_t parent = index;
            size_t child = index;

            const T value = m_Begin[index];

            while (parent < m_Size)
            {
                size_t l = left(parent);
                size_t r = right(parent);

                child = parent;

                if (l < m_Size && m_Comp(m_Begin[l], m_Begin[child]))
                    child = l;

                if (r < m_Size && m_Comp(m_Begin[r], m_Begin[child]))
                    child = r;

                if (child != parent)
                {
                    const T temp = std::move(m_Begin[child]);
                    m_Begin[child] = std::move(m_Begin[parent]);
                    m_Begin[parent] = std::move(temp);

                    parent = child;
                }
                else
                {
                    break;
                }
            }

            m_Begin[child] = value;
        }

    private:
        Alloc m_Alloc;
        size_t m_Size;
        size_t m_Capacity;
        T* m_Begin;
        Comp m_Comp;
    };
}