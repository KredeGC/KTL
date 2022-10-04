#pragma once

#include "alignment_utility.h"

#include <memory>
#include <type_traits>

#include <iostream>

namespace ktl
{
	template<typename T, size_t Size = 4096>
	class freelist_allocator
	{
	private:
		struct free_footer;

	public:
		using value_type = T;
		using size_type = size_t;
		using is_always_equal = std::true_type;

		template<typename U>
		struct rebind
		{
			using other = freelist_allocator<U>;
		};

		freelist_allocator() noexcept
		{
			char* ptr = m_Block;

			ptr += align_to_architecture(size_t(ptr));

			m_Free = reinterpret_cast<free_footer*>(ptr);
			m_Free->AvailableSpace = Size;
			m_Free->Next = nullptr;

			std::cout << "Construct" << std::endl;
			std::cout << (int*)m_Free << std::endl;
			std::cout << (int*)m_Block << std::endl;
			std::cout << (int*)(m_Block + Size + ALIGNMENT - 1) << std::endl;
		}

		freelist_allocator(const freelist_allocator& other) noexcept
		{
			char* ptr = m_Block;

			ptr += align_to_architecture(size_t(ptr));

			m_Free = reinterpret_cast<free_footer*>(ptr);
			m_Free->AvailableSpace = Size;
			m_Free->Next = nullptr;

			std::cout << "Copy" << std::endl;
			std::cout << (int*)m_Free << std::endl;
			std::cout << (int*)m_Block << std::endl;
			std::cout << (int*)(m_Block + Size + ALIGNMENT - 1) << std::endl;
		}

		template<typename U, size_t V>
		freelist_allocator(const freelist_allocator<U, V>& other) noexcept
		{
			char* ptr = m_Block;

			ptr += align_to_architecture(size_t(ptr));

			m_Free = reinterpret_cast<free_footer*>(ptr);
			m_Free->AvailableSpace = Size;
			m_Free->Next = nullptr;

			std::cout << "Copy" << std::endl;
			std::cout << (int*)m_Free << std::endl;
			std::cout << (int*)m_Block << std::endl;
			std::cout << (int*)(m_Block + Size + ALIGNMENT - 1) << std::endl;
		}

		virtual ~freelist_allocator() = default;

		T* allocate(size_type n)
		{
			std::cout << "Allocate" << std::endl;
			std::cout << (int*)m_Free << std::endl;
			std::cout << (int*)m_Block << std::endl;
			std::cout << (int*)(m_Block + Size + ALIGNMENT - 1) << std::endl;

			size_t totalSize = (std::max)(sizeof(T) * n, sizeof(free_footer));
			totalSize += align_to_architecture(totalSize);

			if (totalSize > Size)
				return nullptr;

			if (m_Free == nullptr)
				return nullptr;

			if (!owns(reinterpret_cast<T*>(m_Free)))
			{
				std::cout << "FUUUUUUUUUUUUUUUUUUUUCKKKK!!!!!";
			}

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

			free_footer footer = *current;

			if (footer.AvailableSpace >= totalSize + sizeof(free_footer))
			{
				free_footer* newFooter = reinterpret_cast<free_footer*>(offset + totalSize);

				newFooter->AvailableSpace = footer.AvailableSpace - totalSize;
				newFooter->Next = footer.Next;

				if (parent)
					parent->Next = newFooter;

				if (m_Free == current)
					m_Free = newFooter;
			}
			else
			{
				if (parent)
					parent->Next = footer.Next;

				if (m_Free == current)
					m_Free = footer.Next;
			}

			return reinterpret_cast<T*>(current);
		}

		void deallocate(T* p, size_type n)
		{
			size_t totalSize = (std::max)(sizeof(T) * n, sizeof(free_footer));
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
			return Size / sizeof(T);
		}

		bool owns(T* p)
		{
			char* ptr = reinterpret_cast<char*>(p);
			return ptr >= m_Block && ptr < m_Block + Size + ALIGNMENT - 1;
		}

		void coalesce(free_footer* header)
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
		struct free_footer
		{
			size_t AvailableSpace;
			free_footer* Next;
		};

		char m_Block[Size + ALIGNMENT - 1];
		free_footer* m_Free;
	};

	template<typename T, typename U>
	bool operator==(const freelist_allocator<T>&, const freelist_allocator<U>&) noexcept
	{
		return true;
	}

	template<typename T, typename U>
	bool operator!=(const freelist_allocator<T>&, const freelist_allocator<U>&) noexcept
	{
		return false;
	}
}