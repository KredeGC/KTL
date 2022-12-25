#pragma once

#include "../utility/assert_utility.h"
#include "../utility/alignment_utility.h"
#include "../utility/notomic.h"
#include "list_allocator_fwd.h"
#include "type_allocator.h"

#include <memory>
#include <type_traits>

namespace ktl
{
	template<size_t Size, typename Atomic>
	class list_allocator
	{
	private:
		struct footer
		{
			size_t AvailableSpace;
			footer* Next;
		};

		struct arena
		{
			alignas(ALIGNMENT) char Data[Size];
			Atomic UseCount;
			footer* Free;
			footer* Guess;

			arena() noexcept :
				Data{},
				UseCount(1)
			{
				Free = reinterpret_cast<footer*>(Data);
				Free->AvailableSpace = Size;
				Free->Next = nullptr;

				Guess = Free;
			}
		};

	public:
		list_allocator() noexcept
		{
			m_Block = new arena;
		}

		list_allocator(const list_allocator& other) noexcept :
			m_Block(other.m_Block)
		{
			m_Block->UseCount++;
		}

		list_allocator(list_allocator&& other) noexcept :
			m_Block(other.m_Block)
		{
			KTL_ASSERT(other.m_Block);
			other.m_Block = nullptr;
		}

		~list_allocator()
		{
			if (m_Block)
				decrement();
		}

		list_allocator& operator=(const list_allocator& rhs) noexcept
		{
			if (m_Block)
				decrement();

			m_Block = rhs.m_Block;
			m_Block->UseCount++;

			return *this;
		}

		list_allocator& operator=(list_allocator&& rhs) noexcept
		{
			if (m_Block)
				decrement();

			m_Block = rhs.m_Block;

			rhs.m_Block = nullptr;

			return *this;
		}

		bool operator==(const list_allocator& rhs) const noexcept
		{
			return m_Block == rhs.m_Block;
		}

		bool operator!=(const list_allocator& rhs) const noexcept
		{
			return m_Block != rhs.m_Block;
		}

#pragma region Allocation
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

				if (m_Block->Guess == current)
					m_Block->Guess = current->Next;
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

				coalesce(footerPtr);

				if (m_Block->Guess == begin)
					m_Block->Guess = footerPtr;
			}
			else
			{
				// Utilize the power of random chance
				// Guessing is usually better than starting from scratch
				footer* current = m_Block->Guess < footerPtr ? m_Block->Guess : m_Block->Free;

				//if (m_Block->Guess < footerPtr)
				//	std::cout << footerPtr - m_Block->Guess << std::endl;

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

				m_Block->Guess = current;
			}
		}
#pragma endregion

#pragma region Utility
		size_t max_size() const noexcept
		{
			return Size;
		}

		bool owns(void* p)
		{
			char* ptr = reinterpret_cast<char*>(p);
			return ptr >= m_Block->Data && ptr < m_Block->Data + Size;
		}
#pragma endregion

	private:
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

		void decrement()
		{
			if (m_Block->UseCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
				delete m_Block;
		}

		arena* m_Block;
	};
}