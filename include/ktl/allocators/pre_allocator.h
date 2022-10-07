#pragma once

#include "type_allocator.h"

#include "../utility/alignment_utility.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	// Weirdness: std::vector allocates some data in the constructor with a different allocator (std::_ContainerProxy_t or smth)
	// Question: Why have stateful allocators when copying requires that the state is not altered?
	// Is this just an msc thing? Other compilers seem to work fine, despite *potentially* being UB

	template<size_t Size>
	struct arena
	{
		struct footer
		{
			size_t AvailableSpace;
			footer* Next;
		};

		footer* Free;
		alignas(ALIGNMENT) char Data[Size];

		arena() noexcept :
			Data{}
		{
			Free = reinterpret_cast<footer*>(Data);
			Free->AvailableSpace = Size;
			Free->Next = nullptr;
		}
	};

	template<size_t Size>
	class pre_allocator
	{
	private:
		typedef typename arena<Size>::footer footer;

	public:
		pre_allocator() = delete;

		pre_allocator(arena<Size>& block) noexcept
			: m_Block(&block) {}

		pre_allocator(arena<Size>* block) noexcept
			: m_Block(block) {}

		pre_allocator(const pre_allocator& other) noexcept :
			m_Block(other.m_Block) {}

		void* allocate(size_t n)
		{
			size_t totalSize = (std::max)(n, sizeof(footer));
			totalSize += align_to_architecture(totalSize);

			if (totalSize > Size)
				return nullptr;

			if (m_Block->Free == nullptr)
				return nullptr;

			footer* parent = nullptr;
			footer* current = m_Block->Free;
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

			if (current->AvailableSpace >= totalSize + sizeof(footer))
			{
				footer* newFooter = reinterpret_cast<footer*>(offset + totalSize);

				newFooter->AvailableSpace = current->AvailableSpace - totalSize;
				newFooter->Next = current->Next;

				if (parent)
					parent->Next = newFooter;

				if (m_Block->Free == current)
					m_Block->Free = newFooter;
			}
			else
			{
				if (parent)
					parent->Next = current->Next;

				if (m_Block->Free == current)
					m_Block->Free = current->Next;
			}

			return current;
		}

		void deallocate(void* p, size_t n) noexcept
		{
			size_t totalSize = (std::max)(n, sizeof(footer));
			totalSize += align_to_architecture(totalSize);

			footer* header = reinterpret_cast<footer*>(p);

			header->AvailableSpace = totalSize;

			if (m_Block->Free > header || m_Block->Free == nullptr)
			{
				footer* begin = m_Block->Free;
				header->Next = begin;
				m_Block->Free = header;

				coalesce(m_Block->Free);
			}
			else
			{
				footer* current = m_Block->Free;
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

		size_t max_size() const noexcept
		{
			return Size;
		}

		bool owns(void* p)
		{
			char* ptr = reinterpret_cast<char*>(p);
			return ptr >= m_Block->Data && ptr < m_Block->Data + Size;
		}

		void coalesce(footer* header) noexcept
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

	private:
		arena<Size>* m_Block;
	};

	template<size_t S, size_t T>
	bool operator==(const pre_allocator<S>& lhs, const pre_allocator<T>& rhs) noexcept
	{
		return &lhs == &rhs;
	}

	template<size_t S, size_t T>
	bool operator!=(const pre_allocator<S>& lhs, const pre_allocator<T>& rhs) noexcept
	{
		return &lhs != &rhs;
	}

	template<typename T, size_t Size>
	using type_pre_allocator = type_allocator<T, pre_allocator<Size>>;
}