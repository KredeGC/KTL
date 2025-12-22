#pragma once

#include "../utility/assert.h"
#include "../utility/empty_base.h"
#include "inline_vector_fwd.h"

#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

namespace ktl
{
	template<typename T, size_t Capacity>
	class inline_vector
	{
	private:
		static_assert(std::is_default_constructible<T>::value, "Template class needs to be default constructible");

	public:
		typedef T* iterator;
		typedef const T* const_iterator;

		typedef std::reverse_iterator<T*> reverse_iterator;
		typedef std::reverse_iterator<const T*> const_reverse_iterator;

	public:
		/**
		 * @brief Construct the vector with a default constructed allocator
		*/
		inline_vector() noexcept :
			m_Buffer{},
			m_Size(0) {}

		/**
		 * @brief Construct the vector with the given allocator and initial size
		 * @param n The initial size of the vector
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit inline_vector(size_t n) :
			m_Buffer{},
			m_Size(n)
		{
			KTL_ASSERT(n <= Capacity);

			if constexpr (!std::is_trivial_v<T>)
			{
				for (auto iter = begin(); iter != end(); ++iter)
				{
					new (iter) T();
				}
			}
		}

		/**
		 * @brief Construct the vector with the given allocator, initial size and default value
		 * @param n The initial size of the vector
		 * @param value The value to initialize every element as
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit inline_vector(size_t n, const T& value) :
			m_Buffer{},
			m_Size(n)
		{
			KTL_ASSERT(n <= Capacity);

			std::uninitialized_fill_n<T*, size_t>(reinterpret_cast<T*>(m_Buffer), n, value);
		}

		/**
		 * @brief Construct the vector with the allocator and range of values
		 * @param initializer The initial set of values
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		inline_vector(std::initializer_list<T> initializer) :
			m_Buffer{},
			m_Size(0)
		{
			KTL_ASSERT(initializer.size() <= Capacity);

			for (auto& value : initializer)
			{
				emplace_back(value);
			}
		}

		/**
		 * @brief Construct the vector with the allocator and range of values
		 * @param first A pointer to the first element
		 * @param last A pointer past the last element
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit inline_vector(const T* first, const T* last) :
			m_Buffer{},
			m_Size(last - first)
		{
			if constexpr (std::is_trivial_v<T>)
			{
				size_t n = last - first;

				std::memcpy(m_Buffer, first, n * sizeof(T));
			}
			else
			{
				m_Size = 0;

				for (; first != last; ++first)
				{
					emplace_back(*first);
				}
			}
		}

		inline_vector(const inline_vector& other) noexcept :
			m_Buffer{},
			m_Size(other.m_Size)
		{
			if constexpr (std::is_trivial_v<T>)
			{
				std::memcpy(m_Buffer, other.m_Buffer, other.size() * sizeof(T));
			}
			else
			{
				m_Size = 0;

				for (auto& v : other)
				{
					emplace_back(v);
				}
			}
		}

		inline_vector(inline_vector&& other) noexcept :
			m_Buffer{},
			m_Size(other.m_Size)
		{
			if constexpr (std::is_trivial_v<T>)
			{
				std::memcpy(m_Buffer, other.m_Buffer, other.size() * sizeof(T));
			}
			else
			{
				m_Size = 0;

				for (auto& v : other)
				{
					emplace_back(std::move(v));
					v.~T();
				}
			}

			other.m_Size = 0;
		}

		~inline_vector() noexcept
		{
			if constexpr (!std::is_trivial_v<T>)
			{
				for (auto iter = begin(); iter != end(); ++iter)
				{
					iter->~T();
				}
			}
		}

		inline_vector& operator=(const inline_vector& other) noexcept
		{
			if constexpr (std::is_trivial_v<T>)
			{
				m_Size = other.m_Size;
				std::memcpy(m_Buffer, other.m_Buffer, other.size() * sizeof(T));
			}
			else
			{
				m_Size = 0;

				for (auto& v : other)
				{
					emplace_back(v);
				}
			}

			return *this;
		}

		inline_vector& operator=(inline_vector&& other) noexcept
		{
			if constexpr (std::is_trivial_v<T>)
			{
				m_Size = other.m_Size;
				std::memcpy(m_Buffer, other.m_Buffer, other.size() * sizeof(T));
			}
			else
			{
				m_Size = 0;

				for (auto& v : other)
				{
					emplace_back(std::move(v));
					v.~T();
				}
			}

			other.m_Size = 0;

			return *this;
		}

		friend bool operator==(const inline_vector& lhs, const inline_vector& rhs) noexcept
		{
			if (lhs.size() != rhs.size())
				return false;

			if constexpr (std::is_trivial_v<T>)
			{
				return std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
			}
			else
			{
				for (size_t i = 0; i < lhs.size(); ++i)
				{
					if (lhs.at(i) != rhs.at(i))
						return false;
				}

				return true;
			}
		}

		friend bool operator!=(const inline_vector& lhs, const inline_vector& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the array. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		T& operator[](size_t index) noexcept { KTL_ASSERT(index < size()); return *iterator_at_index(index); }

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the array. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		const T& operator[](size_t index) const noexcept { KTL_ASSERT(index < size()); return *iterator_at_index(index); }


		iterator begin() noexcept { return iterator_at_index(0); }

		const_iterator begin() const noexcept { return iterator_at_index(0); }

		iterator end() noexcept { return iterator_at_index(Capacity); }

		const_iterator end() const noexcept { return iterator_at_index(Capacity); }

		reverse_iterator rbegin() noexcept { return std::reverse_iterator(end()); }

		const_reverse_iterator rbegin() const noexcept { return std::reverse_iterator(end()); }

		reverse_iterator rend() noexcept { return std::reverse_iterator(begin()); }

		const_reverse_iterator rend() const noexcept { return std::reverse_iterator(begin()); }


		/**
		 * @brief Returns the current size of the vector.
		 * @return The current size of the vector in number of elements.
		*/
		size_t size() const noexcept { return m_Size; }

		/**
		 * @brief Returns the current capacity of the vector.
		 * @return The current capacity of the vector in number of elements.
		*/
		size_t capacity() const noexcept { return Capacity; }

		/**
		 * @brief Returns true if the vector has no elements.
		 * @return Whether the vector has a size of 0.
		*/
		bool empty() const noexcept { return m_Size == 0; }


		/**
		 * @brief Returns an iterator to the start of the vector.
		 * @return An iterator to the start of the vector.
		*/
		iterator data() noexcept { return begin(); }

		/**
		 * @brief Returns a const iterator to the start of the vector.
		 * @return A const iterator to the start of the vector.
		*/
		const_iterator data() const noexcept { return begin(); }

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the vector. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		T& at(size_t index) const noexcept { KTL_ASSERT(index < size()); return iterator_at_index(index); }


		/**
		 * @brief Resizes the vector to the given size.
		 * @param n The size to resize to.
		*/
		void resize(size_t n) noexcept
		{
			KTL_ASSERT(n <= Capacity);

			if constexpr (!std::is_trivial_v<T>)
			{
				for (size_t i = m_Size; i < n; ++i)
				{
					new (iterator_at_index(i)) T();
				}
			}

			m_Size = n;
		}

		/**
		 * @brief Pushes a new element into the vector by copying it.
		 * @param value The element to copy into the vector.
		 * @return An iterator to the element that was added.
		*/
		iterator push_back(const T& element) noexcept
		{
			return emplace_back(element);
		}

		/**
		 * @brief Pushes a new element into the vector by moving it.
		 * @param value The element to move into the vector.
		 * @return An iterator to the element that was added.
		*/
		iterator push_back(T&& element) noexcept
		{
			return emplace_back(std::move(element));
		}

		/**
		 * @brief Pushes a range of values into the vector.
		 * @param first A pointer to the first element.
		 * @param last A pointer one element past the last element.
		 * @return An iterator to the element that was added.
		*/
		iterator push_back(const T* first, const T* last) noexcept
		{
			const size_t n = (last - first);

			KTL_ASSERT(m_Size + n <= Capacity);

			T* last_element = end();

			if constexpr (std::is_trivial_v<T>)
			{
				m_Size += n;

				std::memcpy(last_element, first, n * sizeof(T));
			}
			else
			{
				for (; first != last; ++first)
				{
					emplace_back(*first);
				}
			}

			return last_element;
		}

		/**
		 * @brief Pushes a new element into the vector by constructing it.
		 * @tparam ...Args Variadic template arguments.
		 * @param ...args Any arguments to use in the construction of the element.
		 * @return An iterator to the element that was added.
		*/
		template<typename... Args>
		iterator emplace_back(Args&&... args) noexcept
		{
			KTL_ASSERT(m_Size < Capacity);

			T* index = iterator_at_index(m_Size);

			new (index) T(std::forward<Args>(args)...);

			++m_Size;

			return index;
		}

		/**
		 * @brief Inserts a new element into the vector by constructing it.
		 * @tparam ...Args Variadic template arguments.
		 * @param iter An iterator pointing to the location where the new element should be emplaced.
		 * @param ...args Any arguments to use in the construction of the element.
		 * @return An iterator to the element that was added.
		*/
		template<typename... Args>
		void emplace(const_iterator iter, Args&&... args) noexcept
		{
			KTL_ASSERT(iter >= begin() && iter <= end());

			if constexpr (std::is_trivial_v<T>)
			{
				std::memmove(const_cast<iterator>(iter + 1), iter, (end() - iter) * sizeof(T));

				*iter = T(std::forward<Args>(args)...);
				++m_Size;
			}
			else
			{
				// TODO: Move each
			}
		}

		/**
		 * @brief Erases the element pointed to by the iterator.
		 * @param iter An iterator pointing to the element.
		 * @return An iterator pointing to the element immidiately after the erased one.
		*/
		iterator erase(const_iterator iter) noexcept
		{
			KTL_ASSERT(iter >= begin() && iter <= end());

			if constexpr (std::is_trivial_v<T>)
			{
				std::memmove(const_cast<iterator>(iter), iter + 1, ((end() - iter) - 1) * sizeof(T));

				--m_Size;
			}
			else
			{
				for (; iter < end() - 1; ++iter)
				{
					*iter = std::move(iter + 1);
				}
			}

			return const_cast<iterator>(iter);
		}

		/**
		 * @brief Erases all elements in a range.
		 * @param first An iterator pointing to the first element.
		 * @param last An iterator pointing to the location after the last element.
		 * @return An iterator pointing to the element immidiately after the erased ones.
		*/
		iterator erase(const_iterator first, const_iterator last) noexcept
		{
			KTL_ASSERT(first <= last);
			KTL_ASSERT(first >= begin() && last <= end());

			size_t diff = last - first;

			if constexpr (std::is_trivial_v<T>)
			{
				std::memmove(const_cast<iterator>(first), last, (end() - last) * sizeof(T));

				m_Size -= diff;
			}
			else
			{
				for (auto iter = first; iter < end() - diff; ++iter)
				{
					*iter = std::move(iter + diff);
				}
			}

			return const_cast<iterator>(first);
		}

		/**
		 * @brief Removes the last element from the vector and returns it.
		 * @return The last element in the vector.
		*/
		T pop_back() noexcept
		{
			return *iterator_at_index(--m_Size);
		}

		/**
		 * @brief Clears all elements in the vector.
		*/
		void clear() noexcept
		{
			if constexpr (!std::is_trivial_v<T>)
			{
				for (auto iter = begin(); iter != end(); ++iter)
				{
					iter->~T();
				}
			}

			m_Size = 0;
		}

	private:
		T* iterator_at_index(size_t index) noexcept
		{
			return std::launder(reinterpret_cast<T*>(m_Buffer) + index);
		}

		const T* iterator_at_index(size_t index) const noexcept
		{
			return std::launder(reinterpret_cast<const T*>(m_Buffer) + index);
		}

	private:
		alignas(T) uint8_t m_Buffer[Capacity * sizeof(T)];
		size_t m_Size;
	};
}