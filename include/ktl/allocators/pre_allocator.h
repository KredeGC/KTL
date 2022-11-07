#pragma once

#include "type_allocator.h"

#include "../utility/alignment_utility.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Size>
	class pre_allocator
	{
	private:
		struct footer
		{
			size_t AvailableSpace;
			footer* Next;
		};

		struct arena
		{
			footer* Free;
			footer* Guess;
			alignas(ALIGNMENT) char Data[Size];

			arena() noexcept :
				Data{}
			{
				Free = reinterpret_cast<footer*>(Data);
				Free->AvailableSpace = Size;
				Free->Next = nullptr;

				Guess = Free;
			}
		};

	public:
		pre_allocator() noexcept
			: m_Block(std::make_shared<arena>()) {}

		pre_allocator(const pre_allocator& other) noexcept :
			m_Block(other.m_Block) {}

		pre_allocator(pre_allocator&& other) noexcept :
			m_Block(std::move(other.m_Block))
		{
			other.m_Block = nullptr;
		}

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

			// Out-of-memory
			if (current->AvailableSpace < totalSize && !current->Next)
				return nullptr;

			char* offset = reinterpret_cast<char*>(current);

			if (current->AvailableSpace >= totalSize + sizeof(footer))
			{
				footer* footerPtr = reinterpret_cast<footer*>(offset + totalSize);

				footerPtr->AvailableSpace = current->AvailableSpace - totalSize;
				footerPtr->Next = current->Next;

				if (parent)
					parent->Next = footerPtr;

				if (m_Block->Free == current)
					m_Block->Free = footerPtr;

				m_Block->Guess = footerPtr;
			}
			else
			{
				if (parent)
					parent->Next = current->Next;

				if (m_Block->Free == current)
					m_Block->Free = current->Next;

				m_Block->Guess = current;
			}

			return current;
		}

		void deallocate(void* p, size_t n) noexcept
		{
			size_t totalSize = (std::max)(n, sizeof(footer));
			totalSize += align_to_architecture(totalSize);

			footer* footerPtr = reinterpret_cast<footer*>(p);

			footerPtr->AvailableSpace = totalSize;

			if (m_Block->Free > footerPtr || m_Block->Free == nullptr)
			{
				footer* begin = m_Block->Free;
				footerPtr->Next = begin;
				m_Block->Free = footerPtr;

				if (footerPtr->Next)
					coalesce(footerPtr);

				if (footerPtr->Next)
					m_Block->Guess = footerPtr->Next;
				else
					m_Block->Guess = footerPtr;
			}
			else
			{
				// Utilize the power of random chance
				// Guessing is usually better than starting from scratch
				footer* current = m_Block->Guess < footerPtr ? m_Block->Guess : m_Block->Free;

				while (current->Next)
				{
					if (current->Next < footerPtr)
						current = current->Next;
					else
						break;
				}

				footerPtr->Next = current->Next;
				current->Next = footerPtr;

				// Coalesce twice. header's next may be right up against current
				if (current->Next)
				{
					coalesce(current);
					coalesce(current);
				}

				if (current->Next)
					m_Block->Guess = current->Next;
				else
					m_Block->Guess = current;
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

			// Coalesce if the difference in space is less than the size of footer
			if (space < sizeof(footer))
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
		std::shared_ptr<arena> m_Block;
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