#pragma once

#include <ostream>
#include <vector>

namespace ktl
{
    struct trivial_t
    {
        float gCost;
        float hCost;

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

    class complex_t
    {
    public:
        complex_t() noexcept : m_Value(nullptr) {}

        complex_t(double value) noexcept : m_Value(new double[1])
        {
            *m_Value = value;
        }

        complex_t(const complex_t& other) noexcept : m_Value(new double[1])
        {
            *m_Value = *other.m_Value;
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

            m_Value = new double[1];
            *m_Value = *rhs.m_Value;

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