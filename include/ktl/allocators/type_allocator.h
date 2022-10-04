#pragma once

#include "../utility/meta_template.h"

#include <limits>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename T, typename Alloc>
	class type_allocator
	{
		static_assert(has_value_type<Alloc>(), "Building on top of typed allocators is not allowed. Use allocators without a type");

	private:
		template<typename U, typename A>
		friend class type_allocator;

	public:
		using value_type = T;
		using size_type = typename Alloc::size_type;

		template<typename U>
		struct rebind
		{
			using other = type_allocator<U, Alloc>;
		};

		type_allocator(const Alloc& alloc = Alloc()) noexcept :
			m_Alloc(alloc) {}

		type_allocator(const type_allocator& other) noexcept :
			m_Alloc(other.m_Alloc) {}

		template<typename U>
		type_allocator(const type_allocator<U, Alloc>& other) noexcept :
			m_Alloc(other.m_Alloc) {}

		value_type* allocate(size_t n)
		{
			return reinterpret_cast<value_type*>(m_Alloc.allocate(sizeof(value_type) * n));
		}

		void deallocate(value_type* p, size_t n)
		{
			m_Alloc.deallocate(p, sizeof(value_type) * n);
		}

		template<typename... Args>
		void construct(value_type* p, Args&&... args)
		{
			if constexpr (has_no_construct<void, Alloc, value_type*, Args...>::value)
				::new (p) value_type(std::forward<Args>(args)...);
			else
				m_Alloc.construct(p, std::forward<Args>(args)...);
		}

		void destroy(value_type* p)
		{
			if constexpr (has_no_destroy<Alloc, value_type*>::value)
				p->~value_type();
			else
				m_Alloc.destroy(p);
		}

		size_type max_size() const noexcept
		{
			if constexpr (has_no_max_size<Alloc>::value)
				return std::numeric_limits<size_type>::max() / sizeof(T);
			else
				return m_Alloc.max_size() / sizeof(T);
		}

		bool owns(value_type* p)
		{
			if constexpr (has_no_owns<Alloc>::value)
				return false;
			else
				return m_Alloc.owns(reinterpret_cast<value_type*>(p));
		}

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
	bool operator==(const type_allocator<T, Alloc>&, const type_allocator<U, Alloc>&) noexcept
	{
		return true;
	}

	template<typename T, typename U, typename Alloc>
	bool operator!=(const type_allocator<T, Alloc>&, const type_allocator<U, Alloc>&) noexcept
	{
		return false;
	}
}