#pragma once

#include <cstdint>
#include <ostream>

namespace ktl
{
    struct trivial_t
    {
        double gCost;
        double hCost;

        bool operator==(const trivial_t& other) const
        {
            return gCost == other.gCost
                && hCost == other.hCost;
        }

        bool operator<(const trivial_t& other) const
        {
            return (gCost + hCost) < (other.gCost + other.hCost);
        }

        bool operator>(const trivial_t& other) const
        {
            return (gCost + hCost) > (other.gCost + other.hCost);
        }

        friend std::ostream& operator<<(std::ostream& output, const trivial_t& rhs)
        {
            return output << "[g:" << rhs.gCost << ", h:" << rhs.hCost << "]";
        }
    };

    // Has to be bigger than 16, but not aligned
#pragma pack(push, 1)
    struct packed_t
    {
        int* ptr1;
        int* ptr2;
        uint16_t shorty;
        char caca;

        bool operator==(const packed_t& rhs) const
        {
            return ptr1 == rhs.ptr1 && ptr2 == rhs.ptr2 && shorty == rhs.shorty && caca == rhs.caca;
        }

        bool operator<(const packed_t& other) const
        {
            return shorty < other.shorty;
        }

        bool operator>(const packed_t& other) const
        {
            return shorty > other.shorty;
        }

        friend std::ostream& operator<<(std::ostream& output, const packed_t& rhs)
        {
            return output << "[p1:" << rhs.ptr1 << ", p2:" << rhs.ptr2 << ", short:" << rhs.shorty << ", caca:" << rhs.caca << "]";
        }
    };
#pragma pack(pop)

    class complex_t
    {
    public:
        complex_t() noexcept : m_Value(nullptr) {}

        complex_t(double value) noexcept : m_Value(new double[1])
        {
            *m_Value = value;
        }

        complex_t(const complex_t& other) noexcept : m_Value(nullptr)
        {
            if (other.m_Value)
            {
                m_Value = new double[1];
                *m_Value = *other.m_Value;
            }
        }

        complex_t(complex_t&& other) noexcept : m_Value(other.m_Value)
        {
            other.m_Value = nullptr;
        }

        ~complex_t()
        {
            if (m_Value)
                delete[] m_Value;
        }

        complex_t& operator=(const complex_t& rhs) noexcept
        {
            if (m_Value)
                delete[] m_Value;

            if (rhs.m_Value)
            {
                m_Value = new double[1];
                *m_Value = *rhs.m_Value;
            }
            else
            {
                m_Value = nullptr;
            }

            return *this;
        }

        complex_t& operator=(complex_t&& rhs) noexcept
        {
            if (m_Value)
                delete[] m_Value;

            m_Value = rhs.m_Value;
            rhs.m_Value = nullptr;

            return *this;
        }

        bool operator==(const complex_t& other) const
        {
            return *m_Value == *other.m_Value;
        }

        bool operator<(const complex_t& other) const
        {
            return *m_Value < *other.m_Value;
        }

        bool operator>(const complex_t& other) const
        {
            return *m_Value > *other.m_Value;
        }

        friend std::ostream& operator<<(std::ostream& output, const complex_t& rhs)
        {
            return output << "[m_Value:" << rhs.m_Value << "]";
        }

    private:
        double* m_Value;
    };
}