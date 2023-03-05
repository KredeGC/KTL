#pragma once

#include "../utility/meta.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	/**
	 * @brief Wrapper class for making an untyped allocator into a typed allocator
	 * @tparam T The type to use
	 * @tparam Alloc The untyped allocator
	*/
	template<typename T, typename Alloc>
	class type_allocator
	{
	private:
		static_assert(has_no_value_type<Alloc>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(!std::is_const<T>::value, "Using an allocator of const T is ill-formed");

		template<typename U, typename A>
		friend class type_allocator;

	public:
		typedef T value_type;
		typedef typename get_size_type<Alloc>::type size_type;
		typedef std::false_type is_always_equal;

		template<typename U>
		struct rebind
		{
			typedef type_allocator<U, Alloc> other;
		};

		type_allocator() noexcept :
			m_Alloc(Alloc()) {}

		explicit type_allocator(const Alloc& alloc) noexcept :
			m_Alloc(alloc) {}

		explicit type_allocator(Alloc&& alloc) noexcept :
			m_Alloc(std::move(alloc)) {}

		type_allocator(const type_allocator&) noexcept = default;

		type_allocator(type_allocator&&) noexcept = default;

		template<typename U>
		type_allocator(const type_allocator<U, Alloc>& other) noexcept :
			m_Alloc(other.m_Alloc) {}

		type_allocator& operator=(const type_allocator&) noexcept = default;

		type_allocator& operator=(type_allocator&&) noexcept = default;

#pragma region Allocation
		value_type* allocate(size_t n)
		{
			return reinterpret_cast<value_type*>(m_Alloc.allocate(sizeof(value_type) * n));
		}

		void deallocate(value_type* p, size_t n)
		{
			m_Alloc.deallocate(p, sizeof(value_type) * n);
		}
#pragma endregion

#pragma region Construction
		template<typename... Args>
		typename std::enable_if<has_construct<void, Alloc, value_type*, Args...>::value, void>::type
		construct(value_type* p, Args&&... args)
		{
			m_Alloc.construct(p, std::forward<Args>(args)...);
		}

		template<typename A = Alloc>
		typename std::enable_if<has_destroy<A, value_type*>::value, void>::type
		destroy(value_type* p)
		{
			m_Alloc.destroy(p);
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<has_max_size<A>::value, size_type>::type
		max_size() const noexcept
		{
			return m_Alloc.max_size() / sizeof(T);
		}

		template<typename A = Alloc>
		typename std::enable_if<has_owns<A>::value, bool>::type
		owns(value_type* p) const
		{
			return m_Alloc.owns(p);
		}
#pragma endregion

		Alloc& get_allocator()
		{
			return m_Alloc;
		}

		const Alloc& get_allocator() const
		{
			return m_Alloc;
		}

	private:
		Alloc m_Alloc;
	};

	template<typename T, typename U, typename Alloc>
	bool operator==(const type_allocator<T, Alloc>& lhs, const type_allocator<U, Alloc>& rhs) noexcept
	{
		return lhs.get_allocator() == rhs.get_allocator();
	}

	template<typename T, typename U, typename Alloc>
	bool operator!=(const type_allocator<T, Alloc>& lhs, const type_allocator<U, Alloc>& rhs) noexcept
	{
		return lhs.get_allocator() != rhs.get_allocator();
	}
}