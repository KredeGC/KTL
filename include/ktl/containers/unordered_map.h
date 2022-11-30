#pragma once

#include "unordered_map_fwd.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <utility>

namespace ktl
{
	// Insert-only hash map with open addressing
	template<typename K, typename V, typename Hash, typename Equals, typename Alloc>
	class unordered_map
	{
	private:
		static constexpr uint8_t FLAG_OCCUPIED = 0x01;
		static constexpr uint8_t FLAG_DEAD = 0x02;

		struct pair
		{
			K Key;
			V Value;
			uint8_t Flags;

			explicit pair(const K& key) noexcept :
				Key(key),
				Value{},
				Flags(FLAG_OCCUPIED) {}

			explicit pair(const K& key, const V& value) noexcept :
				Key(key),
				Value(value),
				Flags(FLAG_OCCUPIED) {}

			explicit pair(const K& key, V&& value) noexcept :
				Key(key),
				Value(std::move(value)),
				Flags(FLAG_OCCUPIED) {}

			explicit pair(K&& key, V&& value) noexcept :
				Key(std::move(key)),
				Value(std::move(value)),
				Flags(FLAG_OCCUPIED) {}
		};

		class iter
		{
		private:
			friend class unordered_map;

		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = std::pair<K, V>;
			using pointer = std::pair<K, V>*;
			using reference = std::pair<K, V>&;

		public:
			explicit iter(pair* current, pair* end) :
				m_Current(current),
				m_End(end)
			{
				while (m_Current != m_End && (m_Current->Flags & FLAG_OCCUPIED) == 0)
					m_Current++;
			}

			iter& operator++()
			{
				do
				{
					m_Current++;

				} while (m_Current != m_End && (m_Current->Flags & FLAG_OCCUPIED) == 0);

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
				return *reinterpret_cast<std::pair<K, V>*>(m_Current);
			}

			pointer operator->() const noexcept
			{
				return reinterpret_cast<std::pair<K, V>*>(m_Current);
			}

		private:
			pair* m_Current;
			pair* m_End;
		};

		typedef iter iterator;
		typedef const iter const_iterator;

		typedef typename std::allocator_traits<Alloc>::template rebind_alloc<pair> PairAlloc;

		typedef std::allocator_traits<PairAlloc> Traits;

	public:
		unordered_map(const PairAlloc& alloc = PairAlloc()) :
			m_Alloc(alloc),
			m_Begin(nullptr),
			m_End(nullptr),
			m_Count(0),
			m_Mask(0) {}

		explicit unordered_map(size_t size, const PairAlloc& alloc = PairAlloc()) :
			m_Alloc(alloc),
			m_Begin(Traits::allocate(m_Alloc, size)),
			m_End(m_Begin + size),
			m_Count(0),
			m_Mask(size - 1)
		{
			// TODO: Assert that size is a power of 2
			std::memset(m_Begin, 0, size * sizeof(pair));
		}

		unordered_map(const unordered_map& other) noexcept :
			m_Alloc(Traits::select_on_container_copy_construction(static_cast<PairAlloc>(other.m_Alloc))),
			m_Begin(Traits::allocate(m_Alloc, other.capacity())),
			m_End(m_Begin + other.capacity()),
			m_Count(other.m_Count),
			m_Mask(other.m_Mask)
		{
			std::memset(m_Begin, 0, other.capacity() * sizeof(pair));

			for (size_t i = 0; i < other.capacity(); i++)
			{
				pair* block = other.m_Begin + i;

				// Only copy occupied slots
				if ((block->Flags & FLAG_OCCUPIED) != 0)
				{
					pair* dest = m_Begin + i;

					// Copy construct if not dead
					if ((block->Flags & FLAG_DEAD) == 0)
						Traits::construct(m_Alloc, dest, *block);

					dest->Flags = block->Flags;
				}
			}
		}

		unordered_map(unordered_map&& other) noexcept :
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

		~unordered_map()
		{
			release();
		}

		unordered_map& operator=(const unordered_map& rhs) noexcept
		{
			release();

			m_Alloc = rhs.m_Alloc;
			m_Begin = Traits::allocate(m_Alloc, rhs.capacity());
			m_End = m_Begin + rhs.capacity();
			m_Count = rhs.m_Count;
			m_Mask = rhs.m_Mask;

			std::memset(m_Begin, 0, rhs.capacity() * sizeof(pair));

			for (size_t i = 0; i < rhs.capacity(); i++)
			{
				pair* block = rhs.m_Begin + i;

				// Only copy occupied slots
				if ((block->Flags & FLAG_OCCUPIED) != 0)
				{
					pair* dest = m_Begin + i;

					// Copy construct if not dead
					if ((block->Flags & FLAG_DEAD) == 0)
						Traits::construct(m_Alloc, dest, *block);

					dest->Flags = block->Flags;
				}
			}

			return *this;
		}

		unordered_map& operator=(unordered_map&& rhs) noexcept
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

		V& operator[](const K& index) noexcept
		{
			expand(1);

			bool create;
			pair* block = get_pair(index, m_Begin, m_Mask, create);

			if (create)
			{
				Traits::construct(m_Alloc, block, index);
				m_Count++;
			}

			return block->Value;
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


		void insert(const K& index, const V& value) noexcept
		{
			expand(1);

			bool create;
			pair* block = get_pair(index, m_Begin, m_Mask, create);

			if (create)
			{
				Traits::construct(m_Alloc, block, index, value);
				m_Count++;
			}
			else
			{
				block->Value = value;
			}
		}

		void insert(const K& index, V&& value) noexcept
		{
			expand(1);

			bool create;
			pair* block = get_pair(index, m_Begin, m_Mask, create);

			if (create)
			{
				Traits::construct(m_Alloc, block, index, std::move(value));
				m_Count++;
			}
			else
			{
				block->Value = std::move(value);
			}
		}

		void insert(K&& index, V&& value) noexcept
		{
			expand(1);

			bool create;
			pair* block = get_pair(index, m_Begin, m_Mask, create);

			if (create)
			{
				Traits::construct(m_Alloc, block, std::move(index), std::move(value));
				m_Count++;
			}
			else
			{
				block->Value = std::move(value);
			}
		}

		void erase(const K& index) noexcept
		{
			bool create;
			pair* block = get_pair(index, m_Begin, m_Mask, create);

			if (!create)
			{
				Traits::destroy(m_Alloc, block);
				block->Flags = FLAG_OCCUPIED | FLAG_DEAD;
				m_Count--;
			}
		}

		void erase(iterator& iter) noexcept
		{
			pair* block = iter.m_Current;

			// If occupied and not dead
			if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
			{
				Traits::destroy(m_Alloc, block);
				block->Flags = FLAG_OCCUPIED | FLAG_DEAD;
				m_Count--;
			}
		}

		iterator find(const K& index)
		{
			if (m_Begin == nullptr)
				return iterator(nullptr, nullptr);

			bool create;
			pair* block = get_pair(index, m_Begin, m_Mask, create);
			if (!create)
				return iterator(block, m_End);

			return iterator(m_End, m_End);
		}

		void clear() noexcept
		{
            m_Count = 0;
            
			if (m_Begin)
			{
				for (pair* block = m_Begin; block != m_End; block++)
				{
			        // If occupied and not dead
			        if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
						Traits::destroy(m_Alloc, block);
				}
                
                std::memset(m_Begin, 0, capacity() * sizeof(pair));
			}
		}

	private:
		void release()
		{
			if (m_Begin)
			{
				for (pair* block = m_Begin; block != m_End; block++)
				{
					// Only destroy alive and occupied slots
					if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
						Traits::destroy(m_Alloc, block);
				}

				Traits::deallocate(m_Alloc, m_Begin, capacity());
			}
		}

		void expand(size_t n)
		{
			if (m_Count >= capacity() / 2)
			{
				size_t alSize = (std::max)(capacity(), 1ULL) * 2;

				set_size(alSize);
			}
		}

		void set_size(size_t n)
		{
			m_Mask = n - 1;

			size_t curSize = (std::min)(capacity(), n);

			pair* alBlock = Traits::allocate(m_Alloc, n);
			std::memset(alBlock, 0, n * sizeof(pair));

			if (m_Begin != nullptr)
			{
				// Rehash every occupied and alive slot into the new allocated block
				for (pair* block = m_Begin; block != m_End; block++)
				{
					if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
					{
						// Find an empty slot in the new allocation
						pair* dest = find_empty(block->Key, alBlock, m_Mask);
                        
                        Traits::construct(m_Alloc, dest, std::move(*block));
                        
                        Traits::destroy(m_Alloc, block);
					}
				}

				Traits::deallocate(m_Alloc, m_Begin, capacity());
			}

			m_Begin = alBlock;
			m_End = m_Begin + n;
		}

		pair* find_empty(const K& index, pair* begin, size_t mask)
		{
			size_t h = Hash()(index);

			pair* block;
			size_t counter = 0;

			do
			{
				block = begin + hash_collision_offet(h, counter, mask);
				counter++;

				// If occupied and not dead, continue
			} while ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0);

			return block;
		}

		pair* get_pair(const K& index, pair* begin, size_t mask, bool& create)
		{
			size_t h = Hash()(index);

			create = false;
			pair* block;
			size_t counter = 0;

			do
			{
				block = begin + hash_collision_offet(h, counter, mask);
				counter++;

				// Increment while occupied & not dead & key mismatch & end not reached
			} while (block != m_End
				&& (block->Flags & FLAG_OCCUPIED) != 0
				&& (block->Flags & FLAG_DEAD) == 0
				&& (!Equals()(block->Key, index)));

			// If this slot is free or dead, create it
			if ((block->Flags & FLAG_OCCUPIED) == 0 || (block->Flags & FLAG_DEAD) != 0)
				create = true;

			// Return when a matching key was found
			return block;
		}

		static constexpr size_t hash_collision_offet(size_t key, size_t counter, size_t size)
		{
			// mix 64 bits
			//key = (~key) + (key << 21); // key = (key << 21) - key - 1;
			//key = key ^ (key >> 24);
			//key = (key + (key << 3)) + (key << 8); // key * 265
			//key = key ^ (key >> 14);
			//key = (key + (key << 2)) + (key << 4); // key * 21
			//key = key ^ (key >> 28);
			//key = key + (key << 31);

			return (key + counter) & size;
		}

	private:
		PairAlloc m_Alloc;
		pair* m_Begin;
		pair* m_End;
		size_t m_Count;
		size_t m_Mask;
	};
}