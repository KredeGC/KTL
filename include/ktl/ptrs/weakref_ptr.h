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

		using control_alloc = typename ref_ptr<T, Alloc>::control_alloc;

	public:
		explicit weakref_ptr(const Alloc& alloc = Alloc()) noexcept :
			m_Block(nullptr) {}

		weakref_ptr(const ref_ptr<T, Alloc>& other) noexcept :
			m_Block(other.m_Block)
		{
			if (m_Block)
				m_Block->WeakCount++;
		}

		weakref_ptr(const weakref_ptr& other) noexcept :
			m_Block(other.m_Block)
		{
			if (m_Block)
				m_Block->WeakCount++;
		}

		weakref_ptr(weakref_ptr&& other) noexcept :
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

		ref_ptr<T, Alloc> lock() const noexcept
		{
			if (m_Block->UseCount > 0)
				return ref_ptr<T>(*this);
			return nullptr;
		}

		size_t use_count() const noexcept { return m_Block ? m_Block->UseCount : 0; }

		bool expired() const noexcept { return !m_Block || m_Block->UseCount == 0; }

		explicit operator bool() const noexcept { return m_Block && m_Block->UseCount > 0; }

		T* get() const noexcept { return &m_Block->Data; }

	private:
		void free()
		{
			if (!m_Block) return;

			if (--m_Block->WeakCount == 0)
				static_cast<control_alloc>(m_Block->Allocator).deallocate(m_Block, 1);
		}

	private:
		control_block* m_Block;
	};
}