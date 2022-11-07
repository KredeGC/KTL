#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename T, typename Alloc>
	class weakref_ptr;

	// An implementation of a reference-counted resource
	// Major differences from std::shared_ptr include:
	// - The resource is embedded in the control block, hopefully improving cache friendliness
	// - No deleter function, instead the allocator is embedded in the control block
	// - No thread safety, apart from atomic counters
	template<typename T, typename Alloc = std::allocator<T>>
	class ref_ptr
	{
	private:
		friend class weakref_ptr<T, Alloc>;

		struct control_block
		{
			Alloc Allocator;
			std::atomic<size_t> UseCount;
			std::atomic<size_t> WeakCount;
			T Data;
		};

		using control_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<control_block>;

	public:
		explicit ref_ptr(const Alloc& alloc = Alloc()) noexcept :
			m_Block(nullptr) {}

		explicit ref_ptr(const T& value, const Alloc& alloc = Alloc()) noexcept
		{
			m_Block = static_cast<control_alloc>(alloc).allocate(1);

			m_Block->Allocator = alloc;
			m_Block->UseCount = 1;
			m_Block->WeakCount = 1;

			std::allocator_traits<Alloc>::construct(const_cast<Alloc&>(alloc), &m_Block->Data, value);
		}

		ref_ptr(const weakref_ptr<T, Alloc>& other) noexcept :
			m_Block(other.m_Block)
		{
			if (m_Block && m_Block->UseCount > 0)
				m_Block->UseCount++;
			else
				m_Block = nullptr;
		}

		ref_ptr(const ref_ptr& other) noexcept :
			m_Block(other.m_Block)
		{
			if (m_Block)
				m_Block->UseCount++;
		}

		ref_ptr(ref_ptr&& other) noexcept :
			m_Block(other.m_Block)
		{
			other.m_Block = nullptr;
		}

		~ref_ptr()
		{
			free();
		}

		ref_ptr& operator=(const ref_ptr& rhs)
		{
			free();

			m_Block = rhs.m_Block;

			if (m_Block)
				m_Block->UseCount++;

			return *this;
		}

		ref_ptr& operator=(ref_ptr&& rhs)
		{
			free();

			m_Block = rhs.m_Block;

			rhs.m_Block = nullptr;

			return *this;
		}

		void reset(const T& value, const Alloc& alloc = Alloc())
		{
			free();

			control_alloc* m_Block = static_cast<control_alloc>(alloc).allocate(1);

			m_Block->Allocator = alloc;
			m_Block->UseCount = 1;
			m_Block->WeakCount = 1;

			std::allocator_traits<Alloc>::construct(const_cast<Alloc&>(alloc), &m_Block->Data, value);
		}

		void reset()
		{
			free();

			m_Block = nullptr;
		}

		size_t use_count() const noexcept { return m_Block ? m_Block->UseCount : 0; }

		explicit operator bool() const noexcept { return m_Block && m_Block->UseCount > 0; }

		T* get() const noexcept { return &m_Block->Data; }

		T& operator*() const noexcept { return m_Block->Data; }

		T* operator->() const noexcept { return &m_Block->Data; }

	private:
		void free()
		{
			if (!m_Block) return;

			if (--m_Block->UseCount == 0)
			{
				m_Block->WeakCount--;
				std::allocator_traits<Alloc>::destroy(m_Block->Allocator, &m_Block->Data);
			}

			if (m_Block->WeakCount == 0)
				static_cast<control_alloc>(m_Block->Allocator).deallocate(m_Block, 1);
		}

	private:
		control_block* m_Block;
	};
}