#pragma once

#include "../utility/assert.h"
#include "../utility/meta.h"
#include "overflow_fwd.h"
#include "type_allocator.h"

#include <cstring>
#include <memory>
#include <ostream>
#include <type_traits>

namespace ktl
{
	template<typename Alloc, std::ostream& Stream>
	class overflow
	{
	private:
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

	private:
		static constexpr int OVERFLOW_PATTERN = 0b01010101010101010101010101010101;
		static constexpr int64_t OVERFLOW_TEST = 0b0101010101010101010101010101010101010101010101010101010101010101;
		static constexpr size_t OVERFLOW_SIZE = 8;

	public:
		overflow() noexcept :
			m_Alloc(),
			m_Allocs(0),
			m_Constructs(0) {}

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			detail::can_construct_v<Alloc, Args...>>>
		explicit overflow(Args&&... args) noexcept :
			m_Alloc(std::forward<Args>(args)...),
			m_Allocs(0),
			m_Constructs(0) {}

		overflow(const overflow&) noexcept = default;

		overflow(overflow&&) noexcept = default;

		~overflow()
		{
			if (m_Allocs != 0 || m_Constructs != 0)
			{
				Stream << "--------MEMORY LEAK DETECTED--------\nAllocator destroyed while having:\n";
				if (m_Allocs > 0)
					Stream << " Leaked memory (" << m_Allocs << " bytes)\n";
				else if (m_Allocs < 0)
					Stream << " Too many frees (" << -m_Allocs << " bytes)\n";

				if (m_Constructs > 0)
					Stream << " Too many constructor calls (" << m_Constructs << ")\n";
				else if (m_Constructs < 0)
					Stream << " Too many destructor calls (" << -m_Constructs << ")\n";
			}
		}

		overflow& operator=(const overflow&) noexcept = default;

		overflow& operator=(overflow&&) noexcept = default;

		bool operator==(const overflow& rhs) const noexcept
		{
			return m_Alloc == rhs.m_Alloc;
		}

		bool operator!=(const overflow& rhs) const noexcept
		{
			return m_Alloc != rhs.m_Alloc;
		}

#pragma region Allocation
		void* allocate(size_type n)
		{
			m_Allocs += n;

			size_type size = n + OVERFLOW_SIZE * 2;
			char* ptr = reinterpret_cast<char*>(m_Alloc.allocate(size));

			if (!ptr)
				return nullptr;

			std::memset(ptr, OVERFLOW_PATTERN, OVERFLOW_SIZE);
			std::memset(ptr + OVERFLOW_SIZE + n, OVERFLOW_PATTERN, OVERFLOW_SIZE);

			return ptr + OVERFLOW_SIZE;
		}

		void deallocate(void* p, size_type n)
		{
			KTL_ASSERT(p != nullptr);

			m_Allocs -= n;

			if (p)
			{
				char* ptr = reinterpret_cast<char*>(p);

				// Check against corruption
				if (std::memcmp(ptr + n, &OVERFLOW_TEST, OVERFLOW_SIZE) != 0)
					Stream << "--------MEMORY CORRUPTION DETECTED--------\nThe area around " << reinterpret_cast<int*>(ptr + n) << " has been modified\n";

				if (std::memcmp(ptr - OVERFLOW_SIZE, &OVERFLOW_TEST, OVERFLOW_SIZE) != 0)
					Stream << "--------MEMORY CORRUPTION DETECTED--------\nThe area around " << reinterpret_cast<int*>(ptr - OVERFLOW_SIZE) << " has been modified\n";

				size_type size = n + OVERFLOW_SIZE * 2;
				m_Alloc.deallocate(ptr - OVERFLOW_SIZE, size);
			}
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		void construct(T* p, Args&&... args)
		{
			m_Constructs++;

			if constexpr (detail::has_construct_v<Alloc, T*, Args...>)
				m_Alloc.construct(p, std::forward<Args>(args)...);
			else
				::new(p) T(std::forward<Args>(args)...);
		}

		template<typename T>
		void destroy(T* p)
		{
			m_Constructs--;

			if constexpr (detail::has_destroy_v<Alloc, T*>)
				m_Alloc.destroy(p);
			else
				p->~T();
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<detail::has_max_size_v<A>, size_type>::type
		max_size() const noexcept
		{
			return m_Alloc.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<detail::has_owns_v<A>, bool>::type
		owns(void* p) const
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
		int64_t m_Allocs;
		int64_t m_Constructs;
	};
}