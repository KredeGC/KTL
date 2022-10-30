#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename T, typename Alloc>
	class weakref_ptr;

	// An implementation of a shared reference-counted pointer
	// Major differences from std::shared_ptr include:
	// - Small control block
	// - No deleter function, instead the allocator is embedded
	template<typename T, typename Alloc = std::allocator<T>>
	class ref_ptr
	{
	private:
		friend class weakref_ptr<T, Alloc>;

		template<typename U, typename A, typename ...Args>
		friend ref_ptr<U, A> allocate_ref(A& alloc, Args&& ...args);

		template<typename U, typename ...Args>
		friend ref_ptr<U> make_ref(Args&& ...args);

		struct control_block
		{
			size_t UseCount;
			size_t WeakCount;
			T* Data;
		};

		struct control_data
		{
			size_t UseCount;
			size_t WeakCount;
			T* Data;
			T Buffer;
		};

		using control_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<control_block>;
		using data_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<control_data>;

	public:
		explicit ref_ptr(const Alloc& alloc = Alloc()) noexcept :
			m_Alloc(alloc),
			m_Block(nullptr) {}

		explicit ref_ptr(T* ptr, const Alloc& alloc = Alloc()) noexcept :
			m_Alloc(alloc)
		{
			m_Block = static_cast<control_alloc>(m_Alloc).allocate(1);

			m_Block->UseCount = 1;
			m_Block->WeakCount = 1;
			m_Block->Data = ptr;
		}

		explicit ref_ptr(const T& other, const Alloc& alloc = Alloc()) noexcept :
			m_Alloc(alloc)
		{
			control_data* ptr = static_cast<data_alloc>(alloc).allocate(1);

			ptr->UseCount = 1;
			ptr->WeakCount = 1;
			ptr->Data = &ptr->Buffer;

			std::allocator_traits<Alloc>::construct(alloc, ptr->Data, other);

			m_Block = reinterpret_cast<control_block*>(ptr);
		}

		ref_ptr(const weakref_ptr<T, Alloc>& other) noexcept :
			m_Alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(static_cast<Alloc>(other))),
			m_Block(other.m_Block)
		{
			if (m_Block && m_Block->UseCount > 0)
				m_Block->UseCount++;
			else
				m_Block = nullptr;
		}

		ref_ptr(const ref_ptr& other) noexcept :
			m_Alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(static_cast<Alloc>(other))),
			m_Block(other.m_Block)
		{
			if (m_Block)
				m_Block->UseCount++;
		}

		ref_ptr(ref_ptr&& other) noexcept :
			m_Alloc(std::move(other.m_Alloc)),
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

		void reset(T* ptr)
		{
			free();

			if (ptr == nullptr)
			{
				m_Block = nullptr;
			}
			else
			{
				m_Block = static_cast<control_alloc>(m_Alloc).allocate(1);
				m_Block->Data = ptr;
				m_Block->UseCount = 1;
			}
		}

		size_t use_count() const noexcept { return m_Block->UseCount; }

		bool unique() const noexcept { return m_Block->UseCount == 1; }

		explicit operator bool() const noexcept { return m_Block; }

		T* get() const noexcept { return &m_Block->Data; }

		T& operator*() const noexcept { return *m_Block->Data; }

		T* operator->() const noexcept { return m_Block->Data; }

	private:
		void free()
		{
			if (!m_Block) return;

			m_Block->UseCount--;

			if (m_Block->UseCount == 0)
			{
				m_Block->WeakCount--;
				std::allocator_traits<Alloc>::destroy(m_Alloc, m_Block->Data);

				// Deallocate data, if it wasn't allocated together with the control block
				if (m_Block->Data != reinterpret_cast<T*>(&m_Block->Data + 1))
					m_Alloc.deallocate(m_Block->Data, 1);
			}

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

	template<typename T, typename Alloc, typename ...Args>
	ref_ptr<T, Alloc> allocate_ref(Alloc& alloc, Args&& ...args)
	{
		using control_block = typename ref_ptr<T, Alloc>::control_block;
		using control_data = typename ref_ptr<T, Alloc>::control_data;
		using data_alloc = typename ref_ptr<T, Alloc>::data_alloc;

		control_data* ptr = static_cast<data_alloc>(alloc).allocate(1);

		ptr->UseCount = 1;
		ptr->WeakCount = 1;
		ptr->Data = &ptr->Buffer;

		std::allocator_traits<Alloc>::construct(alloc, ptr->Data, std::forward<Args>(args)...);

		ref_ptr<T, Alloc> ref(alloc);

		ref.m_Block = reinterpret_cast<control_block*>(ptr);

		return ref;
	}

	template<typename T, typename ...Args>
	ref_ptr<T> make_ref(Args&& ...args)
	{
		using control_block = typename ref_ptr<T>::control_block;
		using control_data = typename ref_ptr<T>::control_data;

		control_data* ptr = new control_data{ 1, 1, nullptr, { std::forward<Args>(args)... } };

		ptr->Data = &ptr->Buffer;

		ref_ptr<T> ref;

		ref.m_Block = reinterpret_cast<control_block*>(ptr);

		return ref;
	}
}