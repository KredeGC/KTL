#pragma once

#include <atomic>

namespace ktl
{
    template<typename T>
    struct notomic
    {
    private:
		static_assert(std::is_trivially_copyable<T>::value, "Template class needs to be trivially copyable");
		static_assert(std::is_copy_constructible<T>::value, "Template class needs to be copy constructible");
		static_assert(std::is_copy_assignable<T>::value, "Template class needs to be copy assignable");
		static_assert(std::is_move_constructible<T>::value, "Template class needs to be move constructible");
		static_assert(std::is_move_assignable<T>::value, "Template class needs to be move assignable");
        
    public:
        notomic() noexcept = default;
        constexpr notomic(T value) noexcept : m_Value(value) {}
        
        notomic(const notomic&) = delete;
        notomic(notomic&&) = delete;
        
        notomic& operator=(const notomic& value) noexcept = delete;
        notomic& operator=(notomic&& value) noexcept = delete;
        
        operator T() const noexcept { return m_Value; }
        
        T operator++() noexcept
        {
            return ++m_Value;
        }
        
        T operator++(int) noexcept
        {
            return m_Value++;
        }
        
        notomic& operator=(T value) noexcept
        {
            m_Value = value;
            return *this;
        }
        
        T fetch_sub(T value, std::memory_order order = std::memory_order_seq_cst ) noexcept
        {
            return m_Value++;
        }
        
    private:
        T m_Value;
    };
}