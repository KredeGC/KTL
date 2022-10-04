#pragma once

#include "type_allocator.h"

#include "../utility/alignment_utility.h"
#include "../utility/stack_type.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	class freelist_allocator
	{
	private:
		struct free_footer;

	public:
		using size_type = size_t;

		template<size_t Size>
		freelist_allocator(stack<Size>& block) noexcept :
			m_Size(Size)
		{
			m_Block = block.Data;

			m_Block += align_to_architecture(size_t(m_Block));

			m_Free = reinterpret_cast<free_footer*>(m_Block);
			m_Free->AvailableSpace = Size;
			m_Free->Next = nullptr;
		}

		freelist_allocator(const freelist_allocator& other) noexcept :
			m_Block(other.m_Block),
			m_Free(other.m_Free),
			m_Size(other.m_Size) {}

		[[nodiscard]] void* allocate(size_type n)
		{
			size_t totalSize = (std::max)(n, sizeof(free_footer));
			totalSize += align_to_architecture(totalSize);

			if (totalSize > m_Size)
				return nullptr;

			if (m_Free == nullptr)
				return nullptr;

			free_footer* parent = nullptr;
			free_footer* current = m_Free;
			while (current->Next)
			{
				if (current->AvailableSpace < totalSize)
				{
					// Current doesn't have enough available space
					parent = current;
					current = current->Next;
				}
				else
				{
					// Current has space between it and the next
					break;
				}
			}

			if (current->AvailableSpace < totalSize && !current->Next)
				return nullptr;

			char* offset = reinterpret_cast<char*>(current);

			//free_footer footer = *current;

			if (current->AvailableSpace >= totalSize + sizeof(free_footer))
			{
				free_footer* newFooter = reinterpret_cast<free_footer*>(offset + totalSize);

				newFooter->AvailableSpace = current->AvailableSpace - totalSize;
				newFooter->Next = current->Next;

				if (parent)
					parent->Next = newFooter;

				if (m_Free == current)
					m_Free = newFooter;
			}
			else
			{
				if (parent)
					parent->Next = current->Next;

				if (m_Free == current)
					m_Free = current->Next;
			}

			return current;
		}

		void deallocate(void* p, size_type n) noexcept
		{
			size_t totalSize = (std::max)(n, sizeof(free_footer));
			totalSize += align_to_architecture(totalSize);

			free_footer* header = reinterpret_cast<free_footer*>(p);

			header->AvailableSpace = totalSize;

			if (m_Free > header || m_Free == nullptr)
			{
				free_footer* begin = m_Free;
				header->Next = begin;
				m_Free = header;

				coalesce(m_Free);
			}
			else
			{
				free_footer* current = m_Free;
				while (current->Next)
				{
					if (current->Next < header)
						current = current->Next;
					else
						break;
				}

				header->Next = current->Next;
				current->Next = header;

				// Coalesce twice. header's next may be right up against current
				coalesce(current);
				coalesce(current);
			}
		}

		size_type max_size() const noexcept
		{
			return m_Size;
		}

		bool owns(void* p)
		{
			char* ptr = reinterpret_cast<char*>(p);
			return ptr >= m_Block && ptr < m_Block + m_Size;
		}

		void coalesce(free_footer* header) noexcept
		{
			char* headerOffset = reinterpret_cast<char*>(header);
			char* nextOffset = reinterpret_cast<char*>(header->Next);

			size_t diff = size_t(nextOffset - headerOffset);
			size_t space = (diff - header->AvailableSpace);

			if (space == 0)
			{
				header->AvailableSpace = diff;

				if (header->Next)
				{
					header->AvailableSpace += header->Next->AvailableSpace;
					header->Next = header->Next->Next;
				}
			}
		}

		bool operator==(const freelist_allocator& rhs) noexcept
		{
			return m_Block == rhs.m_Block;
		}

		bool operator!=(const freelist_allocator& rhs) noexcept
		{
			return m_Block != rhs.m_Block;
		}

	private:
		struct free_footer
		{
			size_t AvailableSpace;
			free_footer* Next;
		};

		char* m_Block;
		free_footer* m_Free;

		size_t m_Size;
	};

	template<typename T>
	using freelist_type_allocator = type_allocator<T, freelist_allocator>;
}