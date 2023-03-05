#pragma once

#include "../utility/assert.h"
#include "trivial_array_fwd.h"

#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

namespace ktl
{
	/**
	 * @brief A dynamically alloacted array of trivial types
	 * @tparam T The type to use. Must be trivially copyable and default constructible
	 * @tparam Alloc The type of allocoator to use
	*/
	template<typename T, typename Alloc>
	class trivial_array
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
		 * @brief Construct the array with the given allocator
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		trivial_array(const Alloc& allocator = Alloc()) noexcept :
			m_Alloc(allocator),
			m_Begin(nullptr),
			m_End(nullptr) {}

		/**
		 * @brief Construct the array with the given allocator and size
		 * @param n The size of the array
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit trivial_array(size_t n, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, n)),
			m_End(m_Begin + n) {}

		/**
		 * @brief Construct the array with the given allocator, size and default value
		 * @param n The size of the array
		 * @param value The value to initialize every element as
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit trivial_array(size_t n, const T& value, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, n)),
			m_End(m_Begin + n)
		{
			std::uninitialized_fill_n<T*, size_t>(m_Begin, n, value);
		}

		/**
		 * @brief Construct the array with the allocator and range of values
		 * @param initializer The initial set of values
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		trivial_array(std::initializer_list<T> initializer, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, initializer.size())),
			m_End(m_Begin + initializer.size())
		{
			T* dst = m_Begin;
			for (auto value : initializer)
			{
				*dst = value;
				dst++;
			}
		}

		/**
		 * @brief Construct the array with the allocator and range of values
		 * @param first A pointer to the first element
		 * @param last A pointer past the last element
		 * @param allocator The allocator to use. Will be default constructed if unspecified
		*/
		explicit trivial_array(const T* first, const T* last, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, size_t(last - first))),
			m_End(m_Begin + size_t(last - first))
		{
			size_t n = last - first;
			std::memcpy(m_Begin, first, n * sizeof(T));
		}

		trivial_array(const trivial_array& other) noexcept :
			m_Alloc(Traits::select_on_container_copy_construction(other.m_Alloc)),
			m_Begin(Traits::allocate(m_Alloc, other.size())),
			m_End(m_Begin + other.size())
		{
            if (other.m_Begin != nullptr)
			    std::memcpy(m_Begin, other.m_Begin, other.size() * sizeof(T));
		}

		trivial_array(trivial_array&& other) noexcept :
			m_Alloc(std::move(other.m_Alloc)),
			m_Begin(other.m_Begin),
			m_End(other.m_End)
		{
			other.m_Begin = nullptr;
			other.m_End = nullptr;
		}

		trivial_array(const trivial_array& other, const Alloc& allocator) noexcept :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, other.size())),
			m_End(m_Begin + other.size())
		{
            if (other.m_Begin != nullptr)
			    std::memcpy(m_Begin, other.m_Begin, other.size() * sizeof(T));
		}
        
        trivial_array(trivial_array&& other, const Alloc& allocator) noexcept :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, other.size())),
			m_End(m_Begin + other.size())
		{
            // Moving using a different allocator means we can't just move, we have to reallocate
            if (other.m_Begin != nullptr)
            {
                std::memcpy(m_Begin, other.m_Begin, other.size() * sizeof(T));
                
                Traits::deallocate(other.m_Alloc, other.m_Begin, other.size() * sizeof(T));
            }
            
			other.m_Begin = nullptr;
			other.m_End = nullptr;
		}

		~trivial_array() noexcept
		{
			if (m_Begin != nullptr)
				Traits::deallocate(m_Alloc, m_Begin, size());
		}

		trivial_array& operator=(const trivial_array& other) noexcept
		{
			if (m_Begin != nullptr)
				Traits::deallocate(m_Alloc, m_Begin, size());

			m_Alloc = other.m_Alloc;

			size_t n = other.size();

			T* alBlock = Traits::allocate(m_Alloc, n);

			m_Begin = alBlock;
			m_End = m_Begin + n;

			std::memcpy(m_Begin, other.m_Begin, n * sizeof(T));
			return *this;
		}

		trivial_array& operator=(trivial_array&& other) noexcept
		{
			if (m_Begin != nullptr)
				Traits::deallocate(m_Alloc, m_Begin, size());

			m_Alloc = std::move(other.m_Alloc);
			m_Begin = other.m_Begin;
			m_End = other.m_End;

			other.m_Begin = nullptr;
			other.m_End = nullptr;
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

		std::reverse_iterator<T*> rbegin() noexcept { return std::reverse_iterator(m_End); }

		std::reverse_iterator<const T*> rbegin() const noexcept { return std::reverse_iterator(m_End); }

		std::reverse_iterator<T*> rend() noexcept { return std::reverse_iterator(m_Begin); }

		std::reverse_iterator<const T*> rend() const noexcept { return std::reverse_iterator(m_Begin); }


		/**
		 * @brief Returns the current size of the array.
		 * @return The size of the array in number of elements.
		*/
		size_t size() const noexcept { return m_End - m_Begin; }

		/**
		 * @brief Returns true if the array has been initialized with no size.
		 * @return Whether the array has a size of 0.
		*/
		bool empty() const noexcept { return m_Begin == m_End; }


		/**
		 * @brief Returns an iterator to the start of the array.
		 * @return An iterator to the start of the array.
		*/
		iterator data() noexcept { return m_Begin; }

		/**
		 * @brief Returns a const iterator to the start of the array.
		 * @return A const iterator to the start of the array.
		*/
		const_iterator data() const noexcept { return m_Begin; }

		/**
		 * @brief Returns a reference to the element at @p index.
		 * @note An index higher than size() will produce undefined behaviour.
		 * @param index The index of the element in the array. Must be less than size().
		 * @return A reference to the element at @p index.
		*/
		T& at(size_t index) const noexcept { KTL_ASSERT(index < size()); return m_Begin[index]; }


		/**
		 * @brief Resizes the array to the given size.
		 * @param n The size to resize to.
		*/
		void resize(size_t n)
		{
			if (size() != n)
			{
				size_t curSize = size();
				T* alBlock = Traits::allocate(m_Alloc, n);

				if (m_Begin != nullptr)
				{
					std::memcpy(alBlock, m_Begin, std::min(curSize, n) * sizeof(T));
					Traits::deallocate(m_Alloc, m_Begin, curSize);
				}

				m_Begin = alBlock;
				m_End = m_Begin + n;
			}
		}

		/**
		 * @brief Assigns the given values from @p first to @p last. It also resizes if the size doesn't match the number of elements.
		 * @param first A pointer to the first element.
		 * @param last A pointer one element past the last element.
		*/
		void assign(const T* first, const T* last)
		{
			const size_t n = last - first;

			if (n != size())
			{
				T* alBlock = Traits::allocate(m_Alloc, n);

				if (m_Begin != nullptr)
					Traits::deallocate(m_Alloc, m_Begin, size());

				m_Begin = alBlock;
				m_End = m_Begin + n;
			}

			std::memcpy(m_Begin, first, n * sizeof(T));
		}

	private:
		Alloc m_Alloc;
		T* m_Begin;
		T* m_End;
	};
}