#pragma once

#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

namespace ktl
{
	template<typename T, typename Alloc = std::allocator<T>>
	class trivial_vector
	{
	private:
		static_assert(std::is_default_constructible<T>::value, "Template class needs to be default constructible");
		static_assert(std::is_trivially_copyable<T>::value, "Template class needs to be trivially copyable");

		typedef std::allocator_traits<Alloc> Traits;

	public:
		typedef T* iterator;
		typedef const T* const_iterator;

	public:
		trivial_vector(const Alloc& allocator = Alloc()) noexcept :
			m_Alloc(allocator),
			m_Begin(nullptr),
			m_End(nullptr),
			m_EndMax(nullptr) {}

		explicit trivial_vector(size_t n, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, n)),
			m_End(m_Begin + n),
			m_EndMax(m_End) {}

		explicit trivial_vector(size_t n, const T& value, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, n)),
			m_End(m_Begin + n),
			m_EndMax(m_End)
		{
			std::uninitialized_fill_n<T*, size_t>(m_Begin, n, value);
		}

		explicit trivial_vector(std::initializer_list<T> initializer, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, initializer.size())),
			m_End(m_Begin + initializer.size()),
			m_EndMax(m_End)
		{
			T* dst = m_Begin;
			// Using a normal auto for loop gives illegal indirection error
			for (auto i = initializer.begin(); i != initializer.end(); i++)
			{
				*dst = *i;
				dst++;
			}
		}

		explicit trivial_vector(const T* first, const T* last, const Alloc& allocator = Alloc()) :
			m_Alloc(allocator),
			m_Begin(Traits::allocate(m_Alloc, size_t(last - first))),
			m_End(m_Begin + size_t(last - first)),
			m_EndMax(m_End)
		{
			size_t n = last - first;
			std::memcpy(m_Begin, first, n * sizeof(T));
		}

		trivial_vector(const trivial_vector& other) noexcept(std::is_nothrow_copy_constructible_v<T>) :
			m_Alloc(Traits::select_on_container_copy_construction(static_cast<Alloc>(other.m_Alloc))),
			m_Begin(Traits::allocate(m_Alloc, other.size())),
			m_End(m_Begin + other.size()),
			m_EndMax(m_End)
		{
			std::memcpy(m_Begin, other.m_Begin, other.size() * sizeof(T));
		}

		trivial_vector(trivial_vector&& other) noexcept(std::is_nothrow_move_constructible_v<T>) :
			m_Alloc(std::move(other.m_Alloc)),
			m_Begin(other.m_Begin),
			m_End(other.m_End),
			m_EndMax(other.m_EndMax)
		{
			other.m_Begin = nullptr;
			other.m_End = nullptr;
			other.m_EndMax = nullptr;
		}

		~trivial_vector() noexcept
		{
			if (m_Begin != nullptr)
				Traits::deallocate(m_Alloc, m_Begin, capacity());
		}

		trivial_vector& operator=(const trivial_vector& other) noexcept(std::is_nothrow_copy_assignable_v<T>)
		{
			size_t n = other.size();

			T* alBlock = Traits::allocate(other.m_Alloc, n);

			if (m_Begin != nullptr)
				Traits::deallocate(m_Alloc, m_Begin, capacity());

			m_Alloc = std::move(other.m_Alloc);
			m_Begin = alBlock;
			m_End = m_Begin + n;
			m_EndMax = m_Begin + n;

			std::memcpy(m_Begin, other.m_Begin, n * sizeof(T));
			return *this;
		}

		trivial_vector& operator=(trivial_vector&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
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

		size_t capacity() const noexcept { return m_EndMax - m_Begin; }

		bool empty() const noexcept { return m_Begin == m_End; }


		T* data() noexcept { return m_Begin; }

		const T* data() const noexcept { return m_Begin; }


		void resize(size_t n)
		{
			if (capacity() < n)
				set_size(n);

			m_End = m_Begin + n;
		}

		void reserve(size_t n)
		{
			if (capacity() < n)
				set_size(n);
		}

		void push_back(const T& element)
		{
			if (m_End == m_EndMax)
				expand(1);
			*m_End = element;
			m_End++;
		}

		void push_back(T&& element)
		{
			if (m_End == m_EndMax)
				expand(1);
			*m_End = std::move(element);
			m_End++;
		}

		void push_back(const T* first, const T* last)
		{
			const size_t n = (last - first);

			if (capacity() - size() < n)
				expand(n);

			std::memcpy(m_End, first, n * sizeof(T));
		}

		template<typename... Args>
		void emplace_back(Args&&... args)
		{
			if (m_End == m_EndMax)
				expand(1);
			*m_End = T(std::forward<Args>(args)...);
			m_End++;
		}

		void pop_back() { m_End--; }

		void clear() { m_End = m_Begin; }

	private:
		void expand(size_t n)
		{
			size_t curCap = capacity();
			size_t alSize = curCap + (std::max)(curCap, n);

			set_size(alSize);
		}

		void set_size(size_t n)
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
		Alloc m_Alloc;
		T* m_Begin;
		T* m_End;
		T* m_EndMax;
	};
}