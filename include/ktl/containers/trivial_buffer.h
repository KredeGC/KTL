#pragma once

#include "../utility/assert.h"

#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

namespace ktl
{
	template<typename T, size_t Size>
	class trivial_buffer
	{
	private:
		static_assert(std::is_default_constructible<T>::value, "Template class needs to be default constructible");
		static_assert(std::is_trivially_copyable<T>::value, "Template class needs to be trivially copyable");

	public:
		typedef T* iterator;
		typedef const T* const_iterator;

		typedef std::reverse_iterator<T*> reverse_iterator;
		typedef std::reverse_iterator<const T*> const_reverse_iterator;

	public:
		/**
		 * @brief Default construct a buffer of trivial types
		*/
		trivial_buffer() noexcept {}

		explicit trivial_buffer(const T& value)
		{
			std::uninitialized_fill_n<T*, size_t>(reinterpret_cast<T*>(m_Data), Size, value);
		}

		trivial_buffer(std::initializer_list<T> initializer)
		{
			KTL_ASSERT(initializer.size() <= Size);

			T* dst = reinterpret_cast<T*>(m_Data);
			for (auto value : initializer)
			{
				*dst = value;
				dst++;
			}
		}

		explicit trivial_buffer(const T* first, const T* last)
		{
			KTL_ASSERT(last - first == Size);

			std::memcpy(m_Data, first, Size * sizeof(T));
		}

		trivial_buffer(const trivial_buffer& other) noexcept
		{
			std::memcpy(m_Data, other.m_Data, Size * sizeof(T));
		}

		trivial_buffer(trivial_buffer&& other) noexcept
		{
			std::memcpy(m_Data, other.m_Data, Size * sizeof(T));
		}

		trivial_buffer& operator=(const trivial_buffer& other) noexcept
		{
			std::memcpy(m_Data, other.m_Data, Size * sizeof(T));
			return *this;
		}

		trivial_buffer& operator=(trivial_buffer&& other) noexcept
		{
			std::memcpy(m_Data, other.m_Data, Size * sizeof(T));
			return *this;
		}

		/**
		 * @brief Returns a reference to the element at @p index, rolling.
		 * @note An index higher than size() will roll back to the start of the buffer.
		 * @param index The index of the element in the buffer.
		 * @return A reference to the element at @p index.
		*/
		T& operator[](size_t index) noexcept { return reinterpret_cast<T*>(m_Data)[index % Size]; }

		/**
		 * @brief Returns a const reference to the element at @p index, rolling.
		 * @note An index higher than size() will roll back to the start of the buffer.
		 * @param index The index of the element in the buffer.
		 * @return A reference to the element at @p index.
		*/
		const T& operator[](size_t index) const noexcept { return reinterpret_cast<T*>(m_Data)[index % Size]; }


		iterator begin() noexcept { return m_Data; }

		const_iterator begin() const noexcept { return m_Data; }

		iterator end() noexcept { return m_Data + Size; }

		const_iterator end() const noexcept { return m_Data + Size; }

		std::reverse_iterator<T*> rbegin() noexcept { return std::reverse_iterator(m_Data + Size); }

		std::reverse_iterator<const T*> rbegin() const noexcept { return std::reverse_iterator(m_Data + Size); }

		std::reverse_iterator<T*> rend() noexcept { return std::reverse_iterator(m_Data); }

		std::reverse_iterator<const T*> rend() const noexcept { return std::reverse_iterator(m_Data); }


		/**
		 * @brief Returns the size of the buffer
		 * @return The size of the buffer in number of elements.
		*/
		constexpr size_t size() const noexcept { return Size; }

		/**
		 * @brief Returns an iterator to the start of the buffer.
		 * @return An iterator to the start of the buffer.
		*/
		iterator data() noexcept { return m_Data; }

		/**
		 * @brief Returns a const iterator to the start of the buffer.
		 * @return A const iterator to the start of the buffer.
		*/
		const_iterator data() const noexcept { return m_Data; }

		/**
		 * @brief Returns a reference to the element at @p index, rolling.
		 * @note An index higher than size() will roll back to the start of the buffer.
		 * @param index The index of the element in the buffer.
		 * @return A reference to the element at @p index.
		*/
		T& at(size_t index) const noexcept { return reinterpret_cast<T*>(m_Data)[index % Size]; }


		/**
		 * @brief Assigns the given values from @p first to @p last.
		 * @param first A pointer to the first element.
		 * @param last A pointer one element past the last element.
		*/
		void assign(const T* first, const T* last)
		{
			KTL_ASSERT(last - first == Size);

			std::memcpy(m_Data, first, Size * sizeof(T));
		}

	private:
		uint8_t m_Data[Size * sizeof(T)];
	};
}