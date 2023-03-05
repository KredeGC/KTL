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
		/**
		 * @brief Attempts to allocate a chunk of memory defined by @p n
		 * @param n The amount of objects to allocate memory for. Not in bytes, but number of T
		 * @return A location in memory that is at least @p n objects big or nullptr if it could not be allocated
		*/
		value_type* allocate(size_t n)
		{
			return reinterpret_cast<value_type*>(m_Alloc.allocate(sizeof(value_type) * n));
		}

		/**
		 * @brief Attempts to deallocate the memory at location @p p
		 * @param p The location in memory to deallocate
		 * @param n The size that was initially allocated
		*/
		void deallocate(value_type* p, size_t n)
		{
			m_Alloc.deallocate(p, sizeof(value_type) * n);
		}
#pragma endregion

#pragma region Construction
		/**
		 * @brief Constructs an object of T with the given @p ...args at the given location
		 * @note Only defined if the underlying allocator defines it
		 * @tparam ...Args The types of the arguments
		 * @param p The location of the object in memory
		 * @param ...args A range of arguments to use to construct the object
		*/
		template<typename... Args>
		typename std::enable_if<has_construct<void, Alloc, value_type*, Args...>::value, void>::type
		construct(value_type* p, Args&&... args)
		{
			m_Alloc.construct(p, std::forward<Args>(args)...);
		}

		/**
		 * @brief Destructs an object of T at the given location
		 * @note Only defined if the underlying allocator defines it
		 * @param p The location of the object in memory
		*/
		template<typename A = Alloc>
		typename std::enable_if<has_destroy<A, value_type*>::value, void>::type
		destroy(value_type* p)
		{
			m_Alloc.destroy(p);
		}
#pragma endregion

#pragma region Utility
		/**
		 * @brief Returns the maximum size that an allocation can be
		 * @note Only defined if the underlying allocator defines it
		 * @return The maximum size an allocation may be
		*/
		template<typename A = Alloc>
		typename std::enable_if<has_max_size<A>::value, size_type>::type
		max_size() const noexcept
		{
			return m_Alloc.max_size() / sizeof(T);
		}

		/**
		 * @brief Returns whether or not the allocator owns the given location in memory
		 * @note Only defined if the underlying allocator defines it
		 * @param p The location of the object in memory
		 * @return Whether the allocator owns @p p
		*/
		template<typename A = Alloc>
		typename std::enable_if<has_owns<A>::value, bool>::type
		owns(value_type* p) const
		{
			return m_Alloc.owns(p);
		}
#pragma endregion

		/**
		 * @brief Returns a reference to the underlying allocator
		 * @return The allocator
		*/
		Alloc& get_allocator()
		{
			return m_Alloc;
		}

		/**
		 * @brief Returns a const reference to the underlying allocator
		 * @return The allocator
		*/
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