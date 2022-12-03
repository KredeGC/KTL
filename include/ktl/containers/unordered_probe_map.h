#pragma once

#include "../utility/assert_utility.h"
#include "unordered_probe_map_fwd.h"

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
	class unordered_probe_map
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
			friend class unordered_probe_map;

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
		unordered_probe_map(const PairAlloc& alloc = PairAlloc()) :
			m_Alloc(alloc),
			m_Begin(nullptr),
			m_End(nullptr),
			m_Count(0),
			m_Mask(0) {}

		explicit unordered_probe_map(size_t size, const PairAlloc& alloc = PairAlloc()) :
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

			std::memset(m_Begin, 0, n * sizeof(pair));
		}

		unordered_probe_map(const unordered_probe_map& other) noexcept :
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

		unordered_probe_map(unordered_probe_map&& other) noexcept :
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

		~unordered_probe_map()
		{
			release();
		}

		unordered_probe_map& operator=(const unordered_probe_map& rhs) noexcept
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

		unordered_probe_map& operator=(unordered_probe_map&& rhs) noexcept
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
			expand();

			pair* block = get_pair(index, m_Begin, m_Mask);

			if ((block->Flags & FLAG_OCCUPIED) == 0 || (block->Flags & FLAG_DEAD) != 0)
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

		float load_factor() const noexcept { return static_cast<float>(m_Count) / static_cast<float>(capacity()); }


		void reserve(size_t size)
		{
			size_t n = size_pow2(size);

			if (capacity() < n)
				set_size(n);
		}

		V& at(const K& index) const
		{
			pair* block = get_pair(index, m_Begin, m_Mask);

			// Assert that the value exists
			KTL_ASSERT((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0);

			return block->Value;
		}

		template<typename Key, typename Value>
		iterator insert(Key&& index, Value&& value) noexcept
		{
			expand();

			// Disallow inserting the same key twice
			// Lookup is more expensive, so only call in debug
			KTL_ASSERT(find(std::forward<Key>(index)) == end());

			pair* block = find_empty(std::forward<Key>(index), m_Begin, m_Mask);

			Traits::construct(m_Alloc, block, std::forward<Key>(index), std::forward<Value>(value));
			m_Count++;

			return iterator(block, m_End);
		}

		void erase(const K& index) noexcept
		{
			pair* block = get_pair(index, m_Begin, m_Mask);

			// If occupied and not dead
			if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
			{
				Traits::destroy(m_Alloc, block);
				block->Flags = FLAG_OCCUPIED | FLAG_DEAD;
				m_Count--;
			}
		}

		iterator erase(const_iterator& iter) noexcept
		{
			pair* block = iter.m_Current;

			// If occupied and not dead
			if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
			{
				Traits::destroy(m_Alloc, block);
				block->Flags = FLAG_OCCUPIED | FLAG_DEAD;
				m_Count--;
			}

			return iterator(block, m_End);
		}

		iterator find(const K& index) const noexcept
		{
			if (m_Begin == nullptr)
				return iterator(nullptr, nullptr);

			pair* block = get_pair(index, m_Begin, m_Mask);

			// If occupied and not dead
			if ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0)
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
		void release() noexcept
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

		pair* find_empty(const K& index, pair* begin, size_t mask) const noexcept
		{
			size_t h = Hash()(index);

			pair* block;
			size_t counter = 0;

			do
			{
				block = begin + hash_collision_offset(h, counter, mask);
				counter++;

				// Increment while occupied and not dead, continue
				// Since we are looking for empty slots, we can reuse dead ones
			} while ((block->Flags & FLAG_OCCUPIED) != 0 && (block->Flags & FLAG_DEAD) == 0);

			return block;
		}

		pair* get_pair(const K& index, pair* begin, size_t mask) const noexcept
		{
			size_t h = Hash()(index);

			pair* block;
			size_t counter = 0;

			do
			{
				block = begin + hash_collision_offset(h, counter, mask);
				counter++;

				// Increment while occupied and key mismatch
				// Leave dead slots alone. This is called a tombstone, since we don't want to tread on it
			} while ((block->Flags & FLAG_OCCUPIED) != 0 && (!Equals()(block->Key, index)));

			// Return when a matching key was found
			return block;
		}

		static constexpr size_t size_pow2(size_t n) noexcept
		{
			n--;
			n |= n >> 1;
			n |= n >> 2;
			n |= n >> 4;
			n |= n >> 8;
			n |= n >> 16;
			n |= n >> 32;
			n += n + 2;

			return n;
		}

		static constexpr size_t hash_collision_offset(size_t key, size_t counter, size_t size) noexcept
		{
			// Linear probing for best cache locality
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