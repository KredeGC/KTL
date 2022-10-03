#pragma once

#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename T, size_t Size = 4096>
	class freelist_allocator
	{
	private:
		struct free_footer;

	public:
		using value_type = T;
		using is_always_equal = std::true_type;

		template<typename U>
		struct rebind
		{
			using other = freelist_allocator<U>;
		};

		freelist_allocator() noexcept = default;

		template<typename U>
		freelist_allocator(const freelist_allocator<U>& other) noexcept {}

		virtual ~freelist_allocator()
		{
			if (m_Block)
				::operator delete(m_Block);
		}

		T* allocate(size_t n)
		{
			const size_t totalSize = (std::max)(sizeof(T) * n, sizeof(free_footer));

			if (totalSize > Size)
				return nullptr;

			if (m_Block == nullptr)
				allocate_block();

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

			if (current->AvailableSpace < totalSize + sizeof(free_footer) && !current->Next)
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

		void deallocate(T* p, size_t n)
		{
			size_t totalSize = (std::max)(sizeof(T) * n, sizeof(free_footer));

			free_footer* header = reinterpret_cast<free_footer*>(p);

			header->AvailableSpace = totalSize;

			if (m_Free > header)
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

		bool owns(T* p)
		{
			return p >= m_Block && p < m_Block + Size;
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

		free_footer* m_Block = nullptr;
		free_footer* m_Free = nullptr;

		void allocate_block()
		{
			m_Block = reinterpret_cast<free_footer*>(::operator new(Size));

			m_Free = m_Block;
			m_Free->AvailableSpace = Size;
			m_Free->Next = nullptr;
		}
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