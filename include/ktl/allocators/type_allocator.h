#pragma once

#include "../utility/empty_base.h"
#include "../utility/meta.h"

#include "type_allocator_fwd.h"

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
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");
		static_assert(!std::is_const_v<T>, "Using an allocator of const T is ill-formed");

		template<typename U, typename A>
		friend class type_allocator;

	public:
		typedef T value_type;
		typedef typename detail::get_size_type_t<Alloc> size_type;
		typedef std::false_type is_always_equal;

		template<typename U>
		struct rebind
		{
			typedef type_allocator<U, Alloc> other;
		};

		/**
		 * @brief Default constructor
		 * @note Only defined if the underlying allocator defines it
		*/
		type_allocator() noexcept = default;

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			detail::can_construct_v<Alloc, Args...>>>
		explicit type_allocator(Args&&... alloc) noexcept :
			m_Alloc(std::forward<Args>(alloc)...) {}

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
		typename std::enable_if<detail::has_construct_v<Alloc, value_type*, Args...>, void>::type
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
		typename std::enable_if<detail::has_destroy_v<A, value_type*>, void>::type
		destroy(value_type* p)
		{
			m_Alloc.destroy(p);
		}
#pragma endregion

#pragma region Utility
		/**
		 * @brief Returns the maximum size that an allocation of objects can be
		 * @note Only defined if the underlying allocator defines it
		 * @return The maximum size an allocation may be. Not in bytes, but number of T
		*/
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
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
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
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
		KTL_EMPTY_BASE Alloc m_Alloc;
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