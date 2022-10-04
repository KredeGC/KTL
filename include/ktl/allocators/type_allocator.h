#pragma once

#include <limits>
#include <memory>
#include <type_traits>

namespace ktl
{
	// value_type
	template<typename Alloc, typename = void>
	struct has_value_type : std::true_type { };

	template<typename T>
	struct has_value_type<T, std::void_t<typename T::value_type>> : std::false_type { };

	// max_size()
	template<typename Alloc, typename = void>
	struct has_no_max_size : std::true_type { };

	template<typename Alloc>
	struct has_no_max_size<Alloc, std::void_t<decltype(std::declval<Alloc&>().max_size())>> : std::false_type {};

	// construct(T*, Args&&...)
	template<typename Void, typename... Types>
	struct has_no_construct : std::true_type {};

	template<typename Alloc, typename Ptr, typename... Args>
	struct has_no_construct<std::void_t<decltype(std::declval<Alloc&>().construct(std::declval<Ptr>(), std::declval<Args>()...))>, Alloc, Ptr, Args...> : std::false_type {};

	// destroy(T*)
	template<typename Alloc, typename Ptr, typename = void>
	struct has_no_destroy : std::true_type {};

	template<typename Alloc, typename Ptr>
	struct has_no_destroy<Alloc, Ptr, std::void_t<decltype(std::declval<Alloc&>().destroy(std::declval<Ptr>()))>> : std::false_type {};

	// owns(void*)
	template<typename Alloc, typename = void>
	struct has_no_owns : std::true_type { };

	template<typename Alloc>
	struct has_no_owns<Alloc, std::void_t<decltype(std::declval<Alloc&>().owns(std::declval<void*>()))>> : std::false_type {};


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