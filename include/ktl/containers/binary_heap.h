#pragma once

#include "binary_heap_fwd.h"

#include <cstddef>
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
            m_Comp = other.m_Comp;
            m_Size = other.m_Size;
            m_Capacity = other.m_Capacity;
            m_Begin = other.m_Begin;

            other.m_Capacity = 0;
            other.m_Size = 0;
            other.m_Begin = nullptr;

            return *this;
        }


        T* data() noexcept { return m_Begin; }

        const T* data() const noexcept { return m_Begin; }

        size_t size() const noexcept { return m_Size; }

        size_t capacity() const noexcept { return m_Capacity; }

        bool empty() const noexcept { return m_Size == 0; }


        void insert(const T& value)
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
        }

        void insert(T&& value)
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
        }

        T pop()
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

        void clear()
        {
            for (size_t i = 0; i < m_Size; i++)
                Traits::destroy(m_Alloc, m_Begin + i);

            m_Size = 0;
        }

    private:
        void expand(size_t n)
        {
            size_t newCapacity = m_Size + (std::max)(m_Size, n);
            T* newData = Traits::allocate(m_Alloc, newCapacity);

            if (m_Begin)
            {
                // Move construct elements
                for (size_t i = 0; i < m_Size; i++)
                    Traits::construct(m_Alloc, newData + i, std::move(m_Begin[i]));

                // Deconstruct elements
                for (size_t i = 0; i < m_Size; i++)
                    Traits::destroy(m_Alloc, m_Begin + i);

                // Deallocate old array
                Traits::deallocate(m_Alloc, m_Begin, m_Capacity);
            }

            m_Capacity = newCapacity;
            m_Begin = newData;
        }

        constexpr size_t parent(size_t index) const
        {
            return (index - 1) / 2;
        }

        constexpr size_t left(size_t index) const
        {
            return index * 2 + 1;
        }

        constexpr size_t right(size_t index) const
        {
            return index * 2 + 2;
        }

        void heapify(size_t index)
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