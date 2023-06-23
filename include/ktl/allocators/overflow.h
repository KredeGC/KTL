#pragma once

#include "../utility/assert.h"
#include "../utility/empty_base.h"
#include "../utility/meta.h"
#include "overflow_fwd.h"
#include "type_allocator.h"

#include <cstring>
#include <memory>
#include <ostream>
#include <type_traits>

namespace ktl
{
	template<typename Alloc, typename Stream>
	class overflow
	{
	private:
		static_assert(detail::has_no_value_type_v<Alloc>, "Building on top of typed allocators is not allowed. Use allocators without a type");

	public:
		typedef typename detail::get_size_type_t<Alloc> size_type;

	private:
		static constexpr unsigned int OVERFLOW_PATTERN = 0b10100101101001011010010110100101;
		static constexpr unsigned char OVERFLOW_TEST = 0b10100101;
		static constexpr size_t OVERFLOW_SIZE = 64;

	public:
		/**
		 * @brief Construct the allocator with a reference to a stream object
		 * @param stream The stream to use when leaks or corruption happens
		*/
		explicit overflow(Stream& stream)
			noexcept(std::is_nothrow_default_constructible_v<Alloc>) :
			m_Stream(stream),
			m_Alloc(),
			m_Allocs(0),
			m_Constructs(0) {}

		/**
		 * @brief Constructor for forwarding any arguments to the underlying allocator
		*/
		template<typename... Args,
			typename = std::enable_if_t<
			std::is_constructible_v<Alloc, Args...>>>
		explicit overflow(Stream& stream, Args&&... args)
			noexcept(std::is_nothrow_constructible_v<Alloc, Args...>) :
			m_Stream(stream),
			m_Alloc(std::forward<Args>(args)...),
			m_Allocs(0),
			m_Constructs(0) {}

		overflow(const overflow&) = default;

		overflow(overflow&&) = default;

		~overflow()
		{
			if (m_Allocs != 0)
			{
				m_Stream << "--------MEMORY LEAK DETECTED--------\nAllocator destroyed while having:\n";
				if (m_Allocs > 0)
					m_Stream << " Allocated memory (" << m_Allocs << " bytes)\n";
				else
					m_Stream << " Too many frees (" << -m_Allocs << " bytes)\n";
			}

			if (m_Constructs != 0)
			{
				m_Stream << "--------POSSIBLE LOGIC ERROR DETECTED--------\nAllocator destroyed while having:\n";
				if (m_Constructs > 0)
					m_Stream << " Too many constructor calls (" << m_Constructs << ")\n";
				else
					m_Stream << " Too many destructor calls (" << -m_Constructs << ")\n";
			}
		}

		overflow& operator=(const overflow&) = default;

		overflow& operator=(overflow&&) = default;

		bool operator==(const overflow& rhs) const
			noexcept(noexcept(m_Alloc == rhs.m_Alloc))
		{
			return m_Alloc == rhs.m_Alloc;
		}

		bool operator!=(const overflow& rhs) const
			noexcept(noexcept(m_Alloc != rhs.m_Alloc))
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
		void* allocate(size_type n)
			noexcept(detail::has_nothrow_allocate_v<Alloc>)
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

		/**
		 * @brief Attempts to deallocate the memory at location @p p
		 * @param p The location in memory to deallocate
		 * @param n The size that was initially allocated
		*/
		void deallocate(void* p, size_type n)
			noexcept(detail::has_nothrow_deallocate_v<Alloc>)
		{
			KTL_ASSERT(p != nullptr);

			m_Allocs -= n;

			if (p)
			{
				unsigned char* ptr = reinterpret_cast<unsigned char*>(p);

				size_t before = 0;
				size_t after = 0;

				// Check against corruption
				for (unsigned char* i = ptr - 1; i >= ptr - OVERFLOW_SIZE; i--)
				{
					if (*i != OVERFLOW_TEST)
						before = ptr - i;
				}

				for (unsigned char* i = ptr + n; i < ptr + n + OVERFLOW_SIZE; i++)
				{
					if (*i != OVERFLOW_TEST)
						after = i - ptr - n + 1;
				}

				if (before || after)
				{
					m_Stream << "--------MEMORY CORRUPTION DETECTED--------\nThe area around " << p << " (" << n << " bytes) has been illegally modified\n";

					if (before)
						m_Stream << " Before (" << before << " bytes)\n";

					if (after)
						m_Stream << " After (" << after << " bytes)\n";
				}

				size_type size = n + OVERFLOW_SIZE * 2;
				m_Alloc.deallocate(ptr - OVERFLOW_SIZE, size);
			}
		}
#pragma endregion

#pragma region Construction
		/**
		 * @brief Constructs an object of T with the given @p ...args at the given location
		 * @note Keeps track of the number of constructions
		 * @tparam ...Args The types of the arguments
		 * @param p The location of the object in memory
		 * @param ...args A range of arguments to use to construct the object
		*/
		template<typename T, typename... Args>
		void construct(T* p, Args&&... args) noexcept(
			(detail::has_construct_v<Alloc, T*, Args...> && detail::has_nothrow_construct_v<Alloc, T*, Args...>) ||
			std::is_nothrow_constructible_v<T, Args...>)
		{
			m_Constructs++;

			if constexpr (detail::has_construct_v<Alloc, T*, Args...>)
				m_Alloc.construct(p, std::forward<Args>(args)...);
			else
				::new(p) T(std::forward<Args>(args)...);
		}

		/**
		 * @brief Destructs an object of T at the given location
		 * @note Keeps track of the number of destructions
		 * @param p The location of the object in memory
		*/
		template<typename T>
		void destroy(T* p) noexcept(
			(detail::has_destroy_v<Alloc, T*>&& detail::has_nothrow_destroy_v<Alloc, T*>) ||
			std::is_nothrow_destructible_v<T>)
		{
			m_Constructs--;

			if constexpr (detail::has_destroy_v<Alloc, T*>)
				m_Alloc.destroy(p);
			else
				p->~T();
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
		 * @brief Return a reference to the stream that will be used when leaks or corruption occur
		 * @return The stream
		*/
		Stream& get_stream() noexcept
		{
			return m_Stream;
		}

		/**
		 * @brief Return a const reference to the stream that will be used when leaks or corruption occur
		 * @return The stream
		*/
		const Stream& get_stream() const noexcept
		{
			return m_Stream;
		}

	private:
		Stream& m_Stream;
		KTL_EMPTY_BASE Alloc m_Alloc;
		int64_t m_Allocs;
		int64_t m_Constructs;
	};
}