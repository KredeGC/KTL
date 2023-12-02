#pragma once

#include "../utility/assert.h"
#include "../utility/empty_base.h"
#include "trivial_vector_fwd.h"

#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

namespace ktl
{
	/**
	 * @brief A dynamically allocated vector or trivial types
	 * @tparam T The type to use. Must be trivially copyable and default constructible
	 * @tparam Alloc The type of allocoator to use
	*/
	template<typename T, typename Alloc>
	class trivial_vector
	{
	private:
		static_assert(std::is_default_constructible<T>::value, "Template class needs to be default constructible");
		static_assert(std::is_trivially_copyable<T>::value, "Template class needs to be trivially copyable");

		typedef std::allocator_traits<Alloc> Traits;

	public:
		typedef T* iterator;
		typedef const T* const_iterator;
        
        typedef std::reverse_iterator<T*> reverse_iterator;
        typedef std::reverse_iterator<const T*> const_reverse_iterator;

	public:
		/**
		 * @brief Construct the vector with a default constructed allocator
		*/
		trivial_vector() noexcept :
			m_Alloc(),
			m_Begin(nullptr),
			m_End(nullptr),
			m_EndMax(nullptr) {}

		/**
		 * @brief Construct the vector with the given allocator
		 * @param allocator The allocator to use
		*/
		explicit trivial_vector(const Alloc& allocator) noexcept :
			m_Alloc(allocator),
			m_Begin(nullptr),
			m_End(nullptr),
			m_EndMax(nullptr) {}

		/**
		 * @brief Construct the vector with the given allocator and initial size
		 * @param n The initial size of the vector
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit trivial_vector(size_t n, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, n)),
			m_End(m_Begin + n),
			m_EndMax(m_End) {}

		/**
		 * @brief Construct the vector with the given allocator, initial size and default value
		 * @param n The initial size of the vector
		 * @param value The value to initialize every element as
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit trivial_vector(size_t n, const T& value, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, n)),
			m_End(m_Begin + n),
			m_EndMax(m_End)
		{
			std::uninitialized_fill_n<T*, size_t>(m_Begin, n, value);
		}

		/**
		 * @brief Construct the vector with the allocator and range of values
		 * @param initializer The initial set of values
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		trivial_vector(std::initializer_list<T> initializer, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, initializer.size())),
			m_End(m_Begin + initializer.size()),
			m_EndMax(m_End)
		{
			T* dst = m_Begin;
			for (auto value : initializer)
			{
				*dst = value;
				dst++;
			}
		}

		/**
		 * @brief Construct the vector with the allocator and range of values
		 * @param first A pointer to the first element
		 * @param last A pointer past the last element
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit trivial_vector(const T* first, const T* last, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, size_t(last - first))),
			m_End(m_Begin + size_t(last - first)),
			m_EndMax(m_End)
		{
			size_t n = last - first;
			std::memcpy(m_Begin, first, n * sizeof(T));
		}

		trivial_vector(const trivial_vector& other) noexcept :
			m_Alloc(Traits::select_on_container_copy_construction(static_cast<Alloc>(other.m_Alloc))),
			m_Begin(Traits::allocate(m_Alloc, other.size())),
			m_End(m_Begin + other.size()),
			m_EndMax(m_End)
		{
            if (other.m_Begin != nullptr)
			    std::memcpy(m_Begin, other.m_Begin, other.size() * sizeof(T));
		}

		trivial_vector(trivial_vector&& other) noexcept :
			m_Alloc(std::move(other.m_Alloc)),
			m_Begin(other.m_Begin),
			m_End(other.m_End),
			m_EndMax(other.m_EndMax)
		{
			other.m_Begin = nullptr;
			other.m_End = nullptr;
			other.m_EndMax = nullptr;
		}

		trivial_vector(const trivial_vector& other, const Alloc& allocator) noexcept :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, other.size())),
			m_End(m_Begin + other.size()),
			m_EndMax(m_End)
		{
			if (other.m_Begin != nullptr)
				std::memcpy(m_Begin, other.m_Begin, other.size() * sizeof(T));
		}

		trivial_vector(trivial_vector&& other, const Alloc& allocator) noexcept :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, other.size())),
			m_End(m_Begin + other.size()),
			m_EndMax(m_End)
		{
            // Moving using a different allocator means we can't just move, we have to reallocate
            if (other.m_Begin != nullptr)
            {
                std::memcpy(m_Begin, other.m_Begin, other.size() * sizeof(T));
                
                Traits::deallocate(other.m_Alloc, other.m_Begin, other.capacity() * sizeof(T));
            }
            
			other.m_Begin = nullptr;
			other.m_End = nullptr;
			other.m_EndMax = nullptr;
		}

		~trivial_vector() noexcept
		{
			if (m_Begin != nullptr)
				Traits::deallocate(m_Alloc, m_Begin, capacity());
		}

		trivial_vector& operator=(const trivial_vector& other) noexcept
		{
			if (m_Begin != nullptr)
				Traits::deallocate(m_Alloc, m_Begin, capacity());

			m_Alloc = other.m_Alloc;

			size_t n = other.size();

			T* alBlock = Traits::allocate(m_Alloc, n);

			m_Begin = alBlock;
			m_End = m_Begin + n;
			m_EndMax = m_Begin + n;

			std::memcpy(m_Begin, other.m_Begin, n * sizeof(T));
			return *this;
		}

		trivial_vector& operator=(trivial_vector&& other) noexcept
		{
			if (m_Begin != nullptr)
				Traits::deallocate(m_Alloc, m_Begin, capacity());

			m_Alloc = std::move(other.m_Alloc);
			m_Begin = other.m_Begin;
			m_End = other.m_End;
			m_EndMax = other.m_EndMax;

			other.m_Begin = nullptr;
			other.m_End = nullptr;
			other.m_EndMax = nullptr;
			return *this;
		}

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the array. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		T& operator[](size_t index) noexcept { KTL_ASSERT(index < size()); return m_Begin[index]; }

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the array. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		const T& operator[](size_t index) const noexcept { KTL_ASSERT(index < size()); return m_Begin[index]; }


		iterator begin() noexcept { return m_Begin; }

		const_iterator begin() const noexcept { return m_Begin; }

		iterator end() noexcept { return m_End; }

		const_iterator end() const noexcept { return m_End; }

		reverse_iterator rbegin() noexcept { return std::reverse_iterator(m_End); }

		const_reverse_iterator rbegin() const noexcept { return std::reverse_iterator(m_End); }

		reverse_iterator rend() noexcept { return std::reverse_iterator(m_Begin); }

		const_reverse_iterator rend() const noexcept { return std::reverse_iterator(m_Begin); }


		/**
		 * @brief Returns the current size of the vector.
		 * @return The current size of the vector in number of elements.
		*/
		size_t size() const noexcept { return m_End - m_Begin; }

		/**
		 * @brief Returns the current capacity of the vector.
		 * @return The current capacity of the vector in number of elements.
		*/
		size_t capacity() const noexcept { return m_EndMax - m_Begin; }

		/**
		 * @brief Returns true if the vector has no elements.
		 * @return Whether the vector has a size of 0.
		*/
		bool empty() const noexcept { return m_Begin == m_End; }


		/**
		 * @brief Returns an iterator to the start of the vector.
		 * @return An iterator to the start of the vector.
		*/
		iterator data() noexcept { return m_Begin; }

		/**
		 * @brief Returns a const iterator to the start of the vector.
		 * @return A const iterator to the start of the vector.
		*/
		const_iterator data() const noexcept { return m_Begin; }

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the vector. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		T& at(size_t index) const noexcept { KTL_ASSERT(index < size()); return m_Begin[index]; }


		/**
		 * @brief Resizes the vector to the given size.
		 * @param n The size to resize to.
		*/
		void resize(size_t n) noexcept
		{
			if (capacity() < n)
				set_size(n);

			m_End = m_Begin + n;
		}

		/**
		 * @brief Reserves the capacity of the vector to @p n, without initializing any elements.
		 * @param n The minimum capacity of the vector.
		*/
		void reserve(size_t n) noexcept
		{
			if (capacity() < n)
				set_size(n);
		}

		/**
		 * @brief Pushes a new element into the vector by copying it.
         * @param value The element to copy into the vector.
         * @return An iterator to the element that was added.
		*/
		iterator push_back(const T& element) noexcept
		{
			if (m_End == m_EndMax)
				expand(1);
			*m_End = element;

			return m_End++;
		}

		/**
		 * @brief Pushes a new element into the vector by moving it.
		 * @param value The element to move into the vector.
		 * @return An iterator to the element that was added.
		*/
		iterator push_back(T&& element) noexcept
		{
			if (m_End == m_EndMax)
				expand(1);
			*m_End = std::move(element);

			return m_End++;
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

			if (size_t(m_EndMax - m_End) < n)
				expand(n);

			T* lastElement = m_End;
			m_End += n;

			std::memcpy(lastElement, first, n * sizeof(T));

			return lastElement;
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
			if (m_End == m_EndMax)
				expand(1);
			*m_End = T(std::forward<Args>(args)...);

			return m_End++;
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
            KTL_ASSERT(iter >= m_Begin && iter <= m_End);
            
			if (m_End == m_EndMax)
				expand(1);
            
            std::memmove(const_cast<iterator>(iter + 1), iter, (m_End - iter) * sizeof(T));
            
			*iter = T(std::forward<Args>(args)...);
			m_End++;
		}
        
        /**
         * @brief Erases the element pointed to by the iterator.
         * @param iter An iterator pointing to the element.
         * @return An iterator pointing to the element immidiately after the erased one.
        */
        iterator erase(const_iterator iter) noexcept
        {
            KTL_ASSERT(iter >= m_Begin && iter < m_End);
            
            std::memmove(const_cast<iterator>(iter), iter + 1, ((m_End - iter) - 1) * sizeof(T));
            
            m_End--;
            
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
            KTL_ASSERT(first >= m_Begin && last <= m_End);
            
            std::memmove(const_cast<iterator>(first), last, (m_End - last) * sizeof(T));
            
            m_End -= (last - first);
            
            return const_cast<iterator>(first);
        }

		/**
		 * @brief Removes the last element from the vector and returns it.
		 * @return The last element in the vector.
		*/
		T pop_back() noexcept { return m_Begin[--m_End]; }

		/**
		 * @brief Clears all elements in the vector.
		*/
		void clear() noexcept { m_End = m_Begin; }

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

			T* alBlock = Traits::allocate(m_Alloc, n);

			if (m_Begin != nullptr)
			{
				std::memcpy(alBlock, m_Begin, curSize * sizeof(T));

				Traits::deallocate(m_Alloc, m_Begin, capacity());
			}

			m_Begin = alBlock;
			m_End = m_Begin + curSize;
			m_EndMax = m_Begin + n;
		}

	private:
		KTL_EMPTY_BASE Alloc m_Alloc;
		T* m_Begin;
		T* m_End;
		T* m_EndMax;
		
		//std::conditional_t<sizeof(T) < 32, uint8_t[32], uint8_t*> m_Data;
	};
}