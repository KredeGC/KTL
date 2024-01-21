#pragma once

#include "../utility/aligned_malloc.h"
#include "../utility/alignment.h"
#include "../utility/empty_base.h"
#include "../utility/meta.h"
#include "global_fwd.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename Alloc>
	class global
	{
	private:
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		global() noexcept {}

		global(const global& other) noexcept {}

		global(global&& other) noexcept {}

		global& operator=(const global& rhs) noexcept
		{
			return *this;
		}

		global& operator=(global&& rhs) noexcept
		{
			s_Alloc = rhs.s_Alloc;

			return *this;
		}

		bool operator==(const global& rhs) const
		{
			return true;
		}

		bool operator!=(const global& rhs) const
		{
			return true;
		}

#pragma region Allocation
		void* allocate(size_t n)
			noexcept(detail::has_nothrow_allocate_v<Alloc>)
		{
			return s_Alloc.allocate(n);
		}

		void deallocate(void* p, size_t n)
			noexcept(detail::has_nothrow_deallocate_v<Alloc>)
		{
			s_Alloc.deallocate(p, n);
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		typename std::enable_if<detail::has_construct_v<Alloc, T*, Args...>, void>::type
		construct(T* p, Args&&... args)
			noexcept(detail::has_nothrow_construct_v<Alloc, T*, Args...>)
		{
			s_Alloc.construct(p, std::forward<Args>(args)...);
		}

		template<typename T>
		typename std::enable_if<detail::has_destroy_v<Alloc, T*>, void>::type
		destroy(T* p)
			noexcept(detail::has_nothrow_destroy_v<Alloc, T*>)
		{
			s_Alloc.destroy(p);
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const
			noexcept(detail::has_nothrow_max_size_v<A>)
		{
			return s_Alloc.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
			noexcept(detail::has_nothrow_owns_v<A>)
		{
			return s_Alloc.owns(p);
		}
#pragma endregion

		void set_allocator(Alloc&& value) noexcept
		{
			s_Alloc = std::move(value);
		}

		Alloc& get_allocator() noexcept
		{
			return s_Alloc;
		}

		const Alloc& get_allocator() const noexcept
		{
			return s_Alloc;
		}

	private:
		static inline Alloc s_Alloc{};
	};
}