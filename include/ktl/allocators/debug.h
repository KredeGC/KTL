#pragma once

#include "../utility/assert.h"
#include "../utility/empty_base.h"
#include "../utility/meta.h"
#include "../utility/source_location.h"
#include "debug_fwd.h"
#include "type_allocator.h"

#include <cstring>
#include <memory>
#include <ostream>
#include <type_traits>

namespace ktl
{
	template<typename Alloc, typename Container>
	class debug
	{
	private:
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

	public:
		/**
		 * @brief Construct the allocator with a reference to a container object
		 * @param container The container to use for populating statistics
		*/
		explicit debug(Container& container)
			noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
			m_Container(container),
			m_Alloc() {}

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			std::is_constructible_v<Alloc, Args...>>>
		explicit debug(Container& container, Args&&... args)
			noexcept(std::is_nothrow_constructible_v<Alloc, Args...>) :
			m_Container(container),
			m_Alloc(std::forward<Args>(args)...) {}

		debug(const debug&) = default;

		debug(debug&&) = default;

		debug& operator=(const debug&) = default;

		debug& operator=(debug&&) = default;

		bool operator==(const debug& rhs) const
			noexcept(detail::has_nothrow_equal_v<Alloc>)
		{
			return m_Alloc == rhs.m_Alloc;
		}

		bool operator!=(const debug& rhs) const
			noexcept(detail::has_nothrow_not_equal_v<Alloc>)
		{
			return m_Alloc != rhs.m_Alloc;
		}

#pragma region Allocation
		/**
		 * @brief Attempts to allocate a chunk of memory defined by @p n
		 * @note Allocates 64 bytes more on either side of the returned address.
		 * This memory will be used for overflow checking.
		 * @param n The amount of bytes to allocate memory for
		 * @return A location in memory that is at least @p n bytes big or nullptr if it could not be allocated
		*/
		void* allocate(size_type n, const source_location source = KTL_SOURCE())
			noexcept(detail::has_nothrow_allocate_v<Alloc>)
		{
			m_Container.push_back({ source.file_name(), source.line(), n });

			return detail::allocate(m_Alloc, n, source);
		}

		/**
		 * @brief Attempts to deallocate the memory at location @p p
		 * @param p The location in memory to deallocate
		 * @param n The size that was initially allocated
		*/
		void deallocate(void* p, size_type n)
			noexcept(detail::has_nothrow_deallocate_v<Alloc>)
		{
			m_Alloc.deallocate(p, n);
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
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args)
			noexcept(detail::has_nothrow_construct_v<Alloc, T*, Args...>)
		{
			m_Alloc.construct(p, std::forward<Args>(args)...);
		}

		/**
		 * @brief Destructs an object of T at the given location
		 * @note Only defined if the underlying allocator defines it
		 * @param p The location of the object in memory
		*/
		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p)
			noexcept(detail::has_nothrow_destroy_v<Alloc, T*>)
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
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const
			noexcept(detail::has_nothrow_max_size_v<A>)
		{
			return m_Alloc.max_size();
		}

		/**
		 * @brief Returns whether or not the allocator owns the given location in memory
		 * @note Only defined if the underlying allocator defines it
		 * @param p The location of the object in memory
		 * @return Whether the allocator owns @p p
		*/
		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
			noexcept(detail::has_nothrow_owns_v<A>)
		{
			return m_Alloc.owns(p);
		}
#pragma endregion

		/**
		 * @brief Returns a reference to the underlying allocator
		 * @return The allocator
		*/
		Alloc& get_allocator() noexcept
		{
			return m_Alloc;
		}

		/**
		 * @brief Returns a const reference to the underlying allocator
		 * @return The allocator
		*/
		const Alloc& get_allocator() const noexcept
		{
			return m_Alloc;
		}

		/**
		 * @brief Return a reference to the container that will be used to accumulate statistics
		 * @return The stream
		*/
		Container& get_container() noexcept
		{
			return m_Container;
		}

		/**
		 * @brief Return a const reference to the container that will be used to accumulate statistics
		 * @return The stream
		*/
		const Container& get_container() const noexcept
		{
			return m_Container;
		}

	private:
		Container& m_Container;
		KTL_EMPTY_BASE Alloc m_Alloc;
	};
}