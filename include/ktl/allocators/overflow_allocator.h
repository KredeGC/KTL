#pragma once

#include "../utility/meta_template.h"
#include "type_allocator.h"

#include <cstring>
#include <memory>
#include <ostream>
#include <type_traits>

namespace ktl
{
	template<typename Alloc, std::ostream& Stream>
	class overflow_allocator
	{
	private:
		static_assert(has_value_type<Alloc>::value, "Building on top of typed allocators is not allowed. Use allocators without a type");

	public:
		typedef typename get_size_type<Alloc>::type size_type;

	private:
		static constexpr int OVERFLOW_PATTERN = 0b01010101010101010101010101010101;
		static constexpr size_t OVERFLOW_SIZE = 64;

		struct stats
		{
			Alloc Allocator;
			size_type Allocs;
			size_type Constructs;

			stats(const Alloc& alloc) :
				Allocator(alloc),
				Allocs(0),
				Constructs(0) {}
		};

	public:
		overflow_allocator(const Alloc& alloc = Alloc()) noexcept :
			m_Stats(std::make_shared<stats>(alloc)) {}

		overflow_allocator(const overflow_allocator& other) noexcept :
			m_Stats(other.m_Stats) {}

		~overflow_allocator()
		{
			// Only assume a leak when we're the last reference
			// Otherwise copies could still be using it, but receive an error
			if (m_Stats.use_count() == 1)
			{
				if (m_Stats->Allocs != 0 || m_Stats->Constructs != 0)
					Stream << "--------MEMORY LEAK DETECTED--------\nAllocator destroyed while having:\n" << m_Stats->Allocs << " Allocations\n" << m_Stats->Constructs << " Constructions\n";
			}
		}

		bool operator==(const overflow_allocator& rhs) const noexcept
		{
			return m_Stats->Allocator == rhs.m_Stats->Allocator;
		}

		bool operator!=(const overflow_allocator& rhs) const noexcept
		{
			return m_Stats->Allocator != rhs.m_Stats->Allocator;
		}

#pragma region Allocation
		void* allocate(size_type n)
		{
			m_Stats->Allocs += n;

			size_type size = n + OVERFLOW_SIZE * 2;
			char* ptr = reinterpret_cast<char*>(m_Stats->Allocator.allocate(size));

			if (!ptr)
				return nullptr;

			std::memset(ptr, OVERFLOW_PATTERN, OVERFLOW_SIZE);
			std::memset(ptr + OVERFLOW_SIZE + n, OVERFLOW_PATTERN, OVERFLOW_SIZE);

			return ptr + OVERFLOW_SIZE;
		}

		void deallocate(void* p, size_type n)
		{
			m_Stats->Allocs -= n;

			if (p)
			{
				char* ptr = reinterpret_cast<char*>(p);

				// HACK: In reality this should be compared to the reference directly, but that would require more allocation etc...
				// Instead we just compare them to eachother. If corruption has occurred, it's very unlikely to have corrupted in an identical pattern
				if (std::memcmp(ptr - OVERFLOW_SIZE, ptr + n, OVERFLOW_SIZE) != 0)
					Stream << "--------MEMORY CORRUPTION DETECTED--------\nThe area around " << reinterpret_cast<int*>(p) << " has been modified\n";

				size_type size = n + OVERFLOW_SIZE * 2;
				m_Stats->Allocator.deallocate(ptr - OVERFLOW_SIZE, size);
			}
		}
#pragma endregion

#pragma region Construction
		template<typename T, typename... Args>
		void construct(T* p, Args&&... args)
		{
			m_Stats->Constructs++;

			if constexpr (has_construct<void, Alloc, T*, Args...>::value)
				m_Stats->Allocator.construct(p, std::forward<Args>(args)...);
			else
				::new(p) T(std::forward<Args>(args)...);
		}

		template<typename T>
		void destroy(T* p)
		{
			m_Stats->Constructs--;

			if constexpr (has_destroy<Alloc, T*>::value)
				m_Stats->Allocator.destroy(p);
			else
				p->~T();
		}
#pragma endregion

#pragma region Utility
		template<typename A = Alloc>
		typename std::enable_if<has_max_size<A>::value, size_type>::type
		max_size() const noexcept
		{
			return m_Stats->Allocator.max_size();
		}

		template<typename A = Alloc>
		typename std::enable_if<has_owns<A>::value, bool>::type
		owns(void* p)
		{
			return m_Stats->Allocator.owns(p);
		}
#pragma endregion

		Alloc& get_allocator()
		{
			return m_Stats->Allocator;
		}

		const Alloc& get_allocator() const
		{
			return m_Stats->Allocator;
		}

	private:
		std::shared_ptr<stats> m_Stats;
	};

	template<typename T, typename A, std::ostream& Stream>
	using type_overflow_allocator = type_allocator<T, overflow_allocator<A, Stream>>;
}