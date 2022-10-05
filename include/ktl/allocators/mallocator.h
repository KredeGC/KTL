#pragma once

#include "type_allocator.h"

#include "../utility/alignment_malloc.h"
#include "../utility/alignment_utility.h"

#include <memory>
#include <type_traits>
#include <unordered_set>

namespace ktl
{
	class mallocator
	{
	public:
		using size_type = size_t;

		mallocator() noexcept = default;

		mallocator(const mallocator& other) noexcept : m_Allocs(other.m_Allocs) {}

		[[nodiscard]] void* allocate(size_t n)
		{
			void* ptr = aligned_malloc(n, ALIGNMENT);

			m_Allocs.insert(ptr);

			return ptr;
		}

		void deallocate(void* p, size_t n) noexcept
		{
			m_Allocs.erase(m_Allocs.find(p));

			aligned_free(p);
		}

		bool owns(void* p)
		{
			return m_Allocs.find(p) != m_Allocs.end();
		}

		bool operator==(const mallocator& rhs) noexcept
		{
			return m_Allocs.begin() == rhs.m_Allocs.begin();
		}

		bool operator!=(const mallocator& rhs) noexcept
		{
			return m_Allocs.begin() != rhs.m_Allocs.begin();
		}

	private:
		// TODO: Use std::shared_ptr to ensure copy-ability
		std::unordered_set<void*> m_Allocs;
	};

	template<typename T>
	using type_mallocator = type_allocator<T, mallocator>;
}