#pragma once

#include "../utility/assert.h"
#include "../utility/hashing.h"
#include "unordered_set_fwd.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <utility>

namespace ktl
{
	template<typename T, typename Hash, typename Equals, typename Alloc>
	class unordered_set
	{
	private:
		struct node
		{
			T Value;
			uint8_t Flags;

			explicit node(const T& key) noexcept :
				Value(key),
				Flags(FLAG_OCCUPIED) {}

			explicit node(T&& key) noexcept :
				Value(std::move(key)),
				Flags(FLAG_OCCUPIED) {}
		};

		class iter
		{
		private:
			friend class unordered_set;

		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = T;
			using pointer = T*;
			using reference = T&;

		public:
			explicit iter(node* current, node* end) :
				m_Current(current),
				m_End(end)
			{
				while (m_Current != m_End && ((m_Current->Flags & FLAG_OCCUPIED) == 0 || (m_Current->Flags & FLAG_DEAD) != 0))
					m_Current++;
			}

			iter& operator++()
			{
				do
				{
					m_Current++;

				} while (m_Current != m_End && ((m_Current->Flags & FLAG_OCCUPIED) == 0 || (m_Current->Flags & FLAG_DEAD) != 0));

				return *this;
			}

			iter operator++(int)
			{
				iter value(*this);
				operator++();
				return value;
			}

			bool operator==(const iter& rhs) const noexcept
			{
				return m_Current == rhs.m_Current;
			}

			bool operator!=(const iter& rhs) const noexcept
			{
				return !operator==(rhs);
			}

			reference operator*() const noexcept
			{
				return m_Current->Value;
			}

			pointer operator->() const noexcept
			{
				return &m_Current->Value;
			}

		private:
			node* m_Current;
			node* m_End;
		};

		typedef typename std::allocator_traits<Alloc>::template rebind_alloc<node> PairAlloc;

		typedef std::allocator_traits<PairAlloc> Traits;

	public:
		typedef iter iterator;
		typedef const iter const_iterator;

	public:
		unordered_set(const PairAlloc& alloc = PairAlloc()) :
			m_Alloc(alloc),
			m_Begin(nullptr),
			m_End(nullptr),
			m_Count(0),
			m_Mask(0) {}

		explicit unordered_set(size_t size, const PairAlloc& alloc = PairAlloc()) :
			m_Alloc(alloc),
			m_Begin(nullptr),
			m_End(nullptr),
			m_Count(0),
			m_Mask(0)
		{
			size_t n = size_pow2(size);

			m_Begin = Traits::allocate(m_Alloc, n);
			m_End = m_Begin + n;
			m_Mask = n - 1;

			std::memset(m_Begin, 0, n * sizeof(node));
		}

		unordered_set(const unordered_set& other) noexcept :
			m_Alloc(Traits::select_on_container_copy_construction(static_cast<PairAlloc>(other.m_Alloc))),
			m_Begin(Traits::allocate(m_Alloc, other.capacity())),
			m_End(m_Begin + other.capacity()),
			m_Count(other.m_Count),
			m_Mask(other.m_Mask)
		{
			std::memset(m_Begin, 0, other.capacity() * sizeof(node));

			for (size_t i = 0; i < other.capacity(); i++)
			{
				node* block = other.m_Begin + i;

				// Only copy occupied slots
				if ((block->Flags & FLAG_OCCUPIED) != 0)
				{
					node* dest = m_Begin + i;

					// Copy construct if not dead
					if ((block->Flags & FLAG_DEAD) == 0)
						Traits::construct(m_Alloc, dest, *block);

					dest->Flags = block->Flags;
				}
			}
		}

		unordered_set(unordered_set&& other) noexcept :
			m_Alloc(std::move(other.m_Alloc)),
			m_Begin(other.m_Begin),
			m_End(other.m_End),
			m_Count(other.m_Count),
			m_Mask(other.m_Mask)
		{
			other.m_Begin = nullptr;
			other.m_End = nullptr;
			other.m_Count = 0;
		}

		~unordered_set()
		{
			release();
		}

		unordered_set& operator=(const unordered_set& rhs) noexcept
		{
			release();

			m_Alloc = rhs.m_Alloc;
			m_Begin = Traits::allocate(m_Alloc, rhs.capacity());
			m_End = m_Begin + rhs.capacity();
			m_Count = rhs.m_Count;
			m_Mask = rhs.m_Mask;

			std::memset(m_Begin, 0, rhs.capacity() * sizeof(node));

			for (size_t i = 0; i < rhs.capacity(); i++)
			{
				node* block = rhs.m_Begin + i;

				// Only copy occupied slots
				if ((block->Flags & FLAG_OCCUPIED) != 0)
				{
					node* dest = m_Begin + i;

					// Copy construct if not dead
					if ((block->Flags & FLAG_DEAD) == 0)
						Traits::construct(m_Alloc, dest, *block);

					dest->Flags = block->Flags;
				}
			}

			return *this;
		}

		unordered_set& operator=(unordered_set&& rhs) noexcept
		{
			release();

			m_Alloc = std::move(rhs.m_Alloc);
			m_Begin = rhs.m_Begin;
			m_End = rhs.m_End;
			m_Count = rhs.m_Count;
			m_Mask = rhs.m_Mask;

			rhs.m_Begin = nullptr;
			rhs.m_End = nullptr;
			rhs.m_Count = 0;

			return *this;
		}

		iterator begin() noexcept { return iterator(m_Begin, m_End); }

		const_iterator begin() const noexcept { return iterator(m_Begin, m_End); }

		const_iterator cbegin() const noexcept { return iterator(m_Begin, m_End); }

		iterator end() noexcept { return iterator(m_End, m_End); }

		const_iterator end() const noexcept { return iterator(m_End, m_End); }

		const_iterator cend() const noexcept { return iterator(m_End, m_End); }


		size_t size() const noexcept { return m_Count; }

		size_t capacity() const noexcept { return m_End - m_Begin; }

		bool empty() const noexcept { return m_Count == 0; }

		float load_factor() const noexcept { return static_cast<float>(m_Count) / static_cast<float>(capacity()); }


		void reserve(size_t size)
		{
			size_t n = detail::size_pow2(size);

			if (capacity() < n)
				set_size(n);
		}

		template<typename Value>
		iterator insert(Value&& index) noexcept
		{
			expand();

			// Disallow inserting the same key twice
			// Lookup is more expensive, so only call in debug
			KTL_ASSERT(find(std::forward<Value>(index)) == end());

			node* block = find_empty(std::forward<Value>(index), m_Begin, m_Mask);

			Traits::construct(m_Alloc, block, std::forward<Value>(index));
			m_Count++;

			return iterator(block, m_End);
		}

		size_t erase(const T& index) noexcept
		{
			if (m_Begin == nullptr)
				return 0;

			node* block = get_node(index, m_Begin, m_Mask);

			// If occupied and not dead
			if (block != m_End && (block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
			{
				Traits::destroy(m_Alloc, block);
				block->Flags = FLAG_OCCUPIED | FLAG_DEAD;
				m_Count--;

				return 1;
			}

			return 0;
		}

		iterator erase(const_iterator& iter) noexcept
		{
			node* block = iter.m_Current;

			// If occupied and not dead
			if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
			{
				Traits::destroy(m_Alloc, block);
				block->Flags = FLAG_OCCUPIED | FLAG_DEAD;
				m_Count--;
			}

			return iterator(block, m_End);
		}

		iterator find(const T& index) const noexcept
		{
			if (m_Begin == nullptr)
				return iterator(nullptr, nullptr);

			node* block = get_node(index, m_Begin, m_Mask);

			// If occupied and not dead
			if (block != m_End && (block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
				return iterator(block, m_End);

			return iterator(m_End, m_End);
		}

		bool contains(const T& index) const noexcept
		{
			if (m_Begin == nullptr)
				return false;

			node* block = get_node(index, m_Begin, m_Mask);

			return block != m_End && (block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0;
		}

		void clear() noexcept
		{
			m_Count = 0;

			if (m_Begin)
			{
				for (node* block = m_Begin; block != m_End; block++)
				{
					// If occupied and not dead
					if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
						Traits::destroy(m_Alloc, block);
				}

				std::memset(m_Begin, 0, capacity() * sizeof(node));
			}
		}

	private:
		void release() noexcept
		{
			if (m_Begin)
			{
				for (node* block = m_Begin; block != m_End; block++)
				{
					// Only destroy alive and occupied slots
					if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
						Traits::destroy(m_Alloc, block);
				}

				Traits::deallocate(m_Alloc, m_Begin, capacity());
			}
		}

		void expand() noexcept
		{
			if (m_Count >= capacity() / 2)
			{
				size_t curSize = capacity();
				size_t alSize = (curSize > 0 ? curSize : 1) * 2;

				set_size(alSize);
			}
		}

		void set_size(size_t n) noexcept
		{
			m_Mask = n - 1;

			size_t curSize = (std::min)(capacity(), n);

			node* alBlock = Traits::allocate(m_Alloc, n);
			std::memset(alBlock, 0, n * sizeof(node));

			if (m_Begin != nullptr)
			{
				// Rehash every occupied and alive slot into the new allocated block
				for (node* block = m_Begin; block != m_End; block++)
				{
					if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
					{
						// Find an empty slot in the new allocation
						node* dest = find_empty(block->Value, alBlock, m_Mask);

						Traits::construct(m_Alloc, dest, std::move(*block));

						Traits::destroy(m_Alloc, block);
					}
				}

				Traits::deallocate(m_Alloc, m_Begin, capacity());
			}

			m_Begin = alBlock;
			m_End = m_Begin + n;
		}

		node* find_empty(const T& index, node* begin, size_t mask) const noexcept
		{
			size_t h = Hash()(index);

			node* block;
			size_t counter = 0;

			do
			{
				block = begin + detail::hash_collision_offset(h, counter++, mask);

				// Increment while occupied and not dead, continue
				// Since we are looking for empty slots, we can reuse dead ones
			} while ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0);

			return block;
		}

		node* get_node(const T& index, node* begin, size_t mask) const noexcept
		{
			size_t h = Hash()(index);

			node* block;
			size_t counter = 0;

			do
			{
				block = begin + detail::hash_collision_offset(h, counter++, mask);

				// Increment while occupied and key mismatch
				// Leave dead slots alone. This is called a tombstone, since we don't want to tread on it
			} while (counter < capacity() && (block->Flags & FLAG_OCCUPIED) != 0 && ((block->Flags & FLAG_DEAD) != 0 || !Equals()(block->Value, index)));

			// If nothing matches return end
			if (counter == capacity())
				return m_End;

			// Return when a matching key was found
			return block;
		}

	private:
		PairAlloc m_Alloc;
		node* m_Begin;
		node* m_End;
		size_t m_Count;
		size_t m_Mask;
	};
}