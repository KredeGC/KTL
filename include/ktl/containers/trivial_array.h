#pragma once

#include "trivial_array_fwd.h"

#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

namespace ktl
{
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

	public:
		trivial_array(const Alloc& allocator = Alloc()) noexcept :
			m_Alloc(allocator),
			m_Begin(nullptr),
			m_End(nullptr) {}

		explicit trivial_array(size_t n, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, n)),
			m_End(m_Begin + n) {}

		explicit trivial_array(size_t n, const T& value, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, n)),
			m_End(m_Begin + n)
		{
			std::uninitialized_fill_n<T*, size_t>(m_Begin, n, value);
		}

		explicit trivial_array(std::initializer_list<T> initializer, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, initializer.size())),
			m_End(m_Begin + initializer.size())
		{
			T* dst = m_Begin;
			// Using a normal auto for loop gives illegal indirection error
			for (auto i = initializer.begin(); i != initializer.end(); i++)
			{
				*dst = *i;
				dst++;
			}
		}

		explicit trivial_array(const T* first, const T* last, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, size_t(last - first))),
			m_End(m_Begin + size_t(last - first))
		{
			size_t n = last - first;
			std::memcpy(m_Begin, first, n * sizeof(T));
		}

		trivial_array(const trivial_array& other) noexcept(std::is_nothrow_copy_constructible_v<T>) :
			m_Alloc(Traits::select_on_container_copy_construction(static_cast<Alloc>(other.m_Alloc))),
			m_Begin(Traits::allocate(m_Alloc, other.size())),
			m_End(m_Begin + other.size())
		{
			std::memcpy(m_Begin, other.m_Begin, other.size() * sizeof(T));
		}

		trivial_array(trivial_array&& other) noexcept(std::is_nothrow_move_constructible_v<T>) :
			m_Alloc(std::move(other.m_Alloc)),
			m_Begin(other.m_Begin),
			m_End(other.m_End)
		{
			other.m_Begin = nullptr;
			other.m_End = nullptr;
		}

		~trivial_array() noexcept
		{
			if (m_Begin != nullptr)
				Traits::deallocate(m_Alloc, m_Begin, size());
		}

		trivial_array& operator=(const trivial_array& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
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

		trivial_array& operator=(trivial_array&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
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

		T& operator[](size_t index) noexcept { return m_Begin[index]; }

		const T& operator[](size_t index) const noexcept { return m_Begin[index]; }


		iterator begin() noexcept { return m_Begin; }

		const_iterator begin() const noexcept { return m_Begin; }

		iterator end() noexcept { return m_End; }

		const_iterator end() const noexcept { return m_End; }

		std::reverse_iterator<T*> rbegin() noexcept { return std::reverse_iterator(m_End); }

		std::reverse_iterator<const T*> rbegin() const noexcept { return std::reverse_iterator(m_End); }

		std::reverse_iterator<T*> rend() noexcept { return std::reverse_iterator(m_Begin); }

		std::reverse_iterator<const T*> rend() const noexcept { return std::reverse_iterator(m_Begin); }


		size_t size() const noexcept { return m_End - m_Begin; }

		bool empty() const noexcept { return m_Begin == m_End; }


		T* data() noexcept { return m_Begin; }

		const T* data() const noexcept { return m_Begin; }

		T& at(size_t index) const noexcept { return m_Begin[index]; }


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

		void clear() { m_End = m_Begin; }

	private:
		Alloc m_Alloc;
		T* m_Begin;
		T* m_End;
	};
}