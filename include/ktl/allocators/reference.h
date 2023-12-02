#pragma once

#include "../utility/aligned_malloc.h"
#include "../utility/alignment.h"
#include "../utility/empty_base.h"
#include "../utility/meta.h"
#include "reference_fwd.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename Alloc>
	class reference
	{
	private:
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		explicit reference(Alloc& alloc) noexcept:
			m_Alloc(&alloc) {}

		reference(const reference& other) noexcept :
			m_Alloc(other.m_Alloc) {}

		reference(reference&& other) noexcept :
			m_Alloc(other.m_Alloc) {}

		reference& operator=(const reference& rhs) noexcept
		{
			m_Alloc = rhs.m_Alloc;

			return *this;
		}

		reference& operator=(reference&& rhs) noexcept
		{
			m_Alloc = rhs.m_Alloc;

			return *this;
		}

		bool operator==(const reference& rhs) const
			noexcept(detail::has_nothrow_equal_v<Alloc>)
		{
			return m_Alloc == rhs.m_Alloc && *m_Alloc == *rhs.m_Alloc;
		}

		bool operator!=(const reference& rhs) const
			noexcept(detail::has_nothrow_not_equal_v<Alloc>)
		{
			return m_Alloc != rhs.m_Alloc || *m_Alloc != *rhs.m_Alloc;
		}

#pragma region Allocation
		void* allocate(size_t n)
			noexcept(detail::has_nothrow_allocate_v<Alloc>)
		{
			return m_Alloc->allocate(n);
		}

		void deallocate(void* p, size_t n)
			noexcept(detail::has_nothrow_deallocate_v<Alloc>)
		{
			m_Alloc->deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args)
			noexcept(detail::has_nothrow_construct_v<Alloc, T*, Args...>)
		{
			m_Alloc->construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p)
			noexcept(detail::has_nothrow_destroy_v<Alloc, T*>)
		{
			m_Alloc->destroy(p);
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const
			noexcept(detail::has_nothrow_max_size_v<A>)
		{
			return m_Alloc->max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
			noexcept(detail::has_nothrow_owns_v<A>)
		{
			return m_Alloc->owns(p);
		}
#pragma endregion

		Alloc& get_allocator() noexcept
		{
			return *m_Alloc;
		}

		const Alloc& get_allocator() const noexcept
		{
			return *m_Alloc;
		}

	private:
		Alloc* m_Alloc;
	};
}