#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename T, typename Alloc = std::allocator<T>>
	class weakref_ptr
	{
	private:
		friend class ref_ptr<T, Alloc>;

		using control_block = typename ref_ptr<T, Alloc>::control_block;
		using control_data = typename ref_ptr<T, Alloc>::control_data;

		using control_alloc = typename ref_ptr<T, Alloc>::control_alloc;
		using data_alloc = typename ref_ptr<T, Alloc>::data_alloc;

	public:
		explicit weakref_ptr(const Alloc& alloc = Alloc()) noexcept :
			m_Alloc(alloc),
			m_Block(nullptr) {}

		weakref_ptr(const ref_ptr<T, Alloc>& other) noexcept :
			m_Alloc(other.m_Alloc),
			m_Block(other.m_Block)
		{
			if (m_Block)
				m_Block->WeakCount++;
		}

		weakref_ptr(const weakref_ptr& other) noexcept :
			m_Alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(static_cast<Alloc>(other))),
			m_Block(other.m_Block)
		{
			if (m_Block)
				m_Block->WeakCount++;
		}

		weakref_ptr(weakref_ptr&& other) noexcept :
			m_Alloc(std::move(other.m_Alloc)),
			m_Block(other.m_Block)
		{
			other.m_Block = nullptr;
		}

		~weakref_ptr()
		{
			free();
		}

		weakref_ptr& operator=(const weakref_ptr& rhs)
		{
			free();

			m_Block = rhs.m_Block;

			if (m_Block)
				m_Block->WeakCount++;

			return *this;
		}

		weakref_ptr& operator=(weakref_ptr&& rhs)
		{
			free();

			m_Block = rhs.m_Block;

			rhs.m_Block = nullptr;

			return *this;
		}

		void reset()
		{
			free();

			m_Block = nullptr;
		}

		ref_ptr<T> lock() const noexcept
		{
			if (m_Block->UseCount > 0)
				return ref_ptr<T>(*this);
			return nullptr;
		}

		size_t use_count() const noexcept { return m_Block->UseCount; }

		bool expired() const noexcept { return m_Block->UseCount == 0; }

		explicit operator bool() const noexcept { return m_Block && m_Block->UseCount > 0; }

		T* get() const noexcept { return &m_Block->Data; }

	private:
		void free()
		{
			if (!m_Block) return;

			m_Block->WeakCount--;

			if (m_Block->WeakCount == 0)
			{
				// HACK: If Data points to the address right after itself, it was probably allocated together
				// However, there's no guarantee of this as it could have just been allocated in that order, separately
				// The odds of that happening are very unlikely, but it may cause memory corruption when deallocating
				// The only other option would be some kind of flag to tell whether the blocks are separate
				if (m_Block->Data == reinterpret_cast<T*>(&m_Block->Data + 1))
					static_cast<data_alloc>(m_Alloc).deallocate(reinterpret_cast<control_data*>(m_Block), 1);
				else
					static_cast<control_alloc>(m_Alloc).deallocate(m_Block, 1);
			}
		}

	private:
		control_block* m_Block;
		Alloc m_Alloc;
	};
}