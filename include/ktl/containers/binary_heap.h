#pragma once

#include <cstddef>
#include <memory>
#include <utility>

namespace ktl
{
    template<typename T, typename Comp, typename Alloc = std::allocator<T>>
    class binary_heap
    {
    private:
        typedef std::allocator_traits<Alloc> Traits;

    public:
        explicit binary_heap(const Alloc& allocator = Alloc()) noexcept :
            m_Alloc(allocator),
            m_Comp(Comp()),
            m_Data(Traits::allocate(m_Alloc, m_Capacity))
        {
            for (size_t i = 0; i < m_Capacity; i++)
                Traits::construct(m_Alloc, m_Data + i);
        }

        explicit binary_heap(size_t capacity, const Alloc& allocator = Alloc()) noexcept :
            m_Alloc(allocator),
            m_Comp(Comp()),
            m_Capacity(capacity),
            m_Data(Traits::allocate(m_Alloc, capacity))
        {
            for (size_t i = 0; i < m_Capacity; i++)
                Traits::construct(m_Alloc, m_Data + i);
        }

        binary_heap(const binary_heap& other) noexcept(std::is_nothrow_copy_constructible_v<T>) :
            m_Alloc(Traits::select_on_container_copy_construction(static_cast<Alloc>(other))),
            m_Comp(other.m_Comp),
            m_Capacity(other.m_Size),
            m_Size(other.m_Size),
            m_Data(Traits::allocate(m_Alloc, other.m_Size))
        {
            for (size_t i = 0; i < m_Size; i++)
                Traits::construct(m_Alloc, m_Data + i, other.m_Data[i]);
        }

        binary_heap(binary_heap&& other) noexcept(std::is_nothrow_move_constructible_v<T>) :
            m_Alloc(std::move(other)),
            m_Comp(other.m_Comp),
            m_Capacity(other.m_Size),
            m_Size(other.m_Size),
            m_Data(std::move(other.m_Data))
        {
            other.m_Capacity = 0;
            other.m_Size = 0;
            other.m_Data = nullptr;
        }

        ~binary_heap()
        {
            // Deconstruct elements
            if (m_Data)
            {
                for (size_t i = 0; i < m_Capacity; i++)
                    Traits::destroy(m_Alloc, m_Data + i);

                Traits::deallocate(m_Alloc, m_Data, m_Capacity);
            }
        }

        binary_heap& operator=(const binary_heap& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
        {
            if (m_Data)
            {
                for (size_t i = 0; i < m_Capacity; i++)
                    Traits::destroy(m_Alloc, m_Data + i);

                Traits::deallocate(m_Alloc, m_Data, m_Capacity);
            }

            m_Comp = other.m_Comp;
            m_Capacity = other.m_Size;
            m_Size = other.m_Size;
            m_Data = Traits::allocate(m_Alloc, other.m_Size);

            // Construct elements
            {
                for (size_t i = 0; i < m_Size; i++)
                    Traits::construct(m_Alloc, m_Data + i);

                for (size_t i = 0; i < m_Size; i++)
                    m_Data[i] = other.m_Data[i];
            }

            return *this;
        }

        binary_heap& operator=(binary_heap&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
        {
            // Deconstruct elements
            if (m_Data)
            {
                for (size_t i = 0; i < m_Capacity; i++)
                    Traits::destroy(m_Alloc, m_Data + i);

                Traits::deallocate(m_Alloc, m_Data, m_Capacity);
            }

            m_Comp = other.m_Comp;
            m_Capacity = other.m_Size;
            m_Size = other.m_Size;
            m_Data = std::move(other.m_Data);

            other.m_Capacity = 0;
            other.m_Size = 0;
            other.m_Data = nullptr;

            return *this;
        }

        T& operator[](size_t index)
        {
            return *(m_Data + index);
        }

        void insert(const T& value)
        {
            expand();

            size_t hole = m_Size++;
            while (hole != 0 && m_Comp(value, m_Data[parent(hole)]))
            {
                m_Data[hole] = std::move(m_Data[parent(hole)]);
                hole = parent(hole);
            }

            m_Data[hole] = value;
        }

        T pop()
        {
            T root = m_Data[0];

            if (m_Size > 0)
            {
                m_Data[0] = m_Data[--m_Size];
                heapify(0);
            }

            return root;
        }

        T* data() noexcept { return m_Data; }

        const T* data() const noexcept { return m_Data; }

        size_t size() const noexcept { return m_Size; }

        bool empty() const noexcept { return m_Size == 0; }

        void clear() { m_Size = 0; }

    private:
        void expand()
        {
            if (m_Data && m_Size >= m_Capacity)
            {
                size_t newCapacity = m_Capacity * 2 + 1;
                T* newData = Traits::allocate(m_Alloc, newCapacity);

                // Construct elements
                {
                    for (size_t i = 0; i < newCapacity; i++)
                        Traits::construct(m_Alloc, newData + i);

                    for (size_t i = 0; i < m_Size; i++)
                        newData[i] = std::move(m_Data[i]);
                }

                // Deconstruct elements
                {
                    for (size_t i = 0; i < m_Capacity; i++)
                        Traits::destroy(m_Alloc, m_Data + i);

                    Traits::deallocate(m_Alloc, m_Data, m_Capacity);
                }

                m_Capacity = newCapacity;
                m_Data = newData;
            }
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

            const T value = m_Data[index];

            while (parent < m_Size)
            {
                size_t l = left(parent);
                size_t r = right(parent);

                child = parent;

                if (l < m_Size && m_Comp(m_Data[l], m_Data[child]))
                    child = l;

                if (r < m_Size && m_Comp(m_Data[r], m_Data[child]))
                    child = r;

                if (child != parent)
                {
                    const T temp = std::move(m_Data[child]);
                    m_Data[child] = std::move(m_Data[parent]);
                    m_Data[parent] = std::move(temp);

                    parent = child;
                }
                else
                {
                    break;
                }
            }

            m_Data[child] = value;
        }

    private:
        Alloc m_Alloc;
        T* m_Data = nullptr;
        size_t m_Size = 0;
        size_t m_Capacity = 2;
        Comp m_Comp;
    };

    template<typename T, typename Alloc = std::allocator<T>>
    using binary_min_heap = binary_heap<T, std::less<T>, Alloc>;

    template<typename T, typename Alloc = std::allocator<T>>
    using binary_max_heap = binary_heap<T, std::greater<T>, Alloc>;
}