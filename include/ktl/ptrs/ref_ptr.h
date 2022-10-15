#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

namespace ktl
{
	template<typename T, typename Alloc = std::allocator<T>>
	class ref_ptr
	{
	private:
		template<typename U, typename A, typename ...Args>
		friend ref_ptr<U, A> allocate_ref(A& alloc, Args&& ...args);

		struct control_block
		{
			size_t UseCount;
			T* Data;
		};

		using type_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<control_block>;

	public:
		ref_ptr(const Alloc& alloc = Alloc()) noexcept :
			m_Alloc(alloc),
			m_Block(nullptr) {}

		ref_ptr(T* ptr, const Alloc& alloc = Alloc()) noexcept :
			m_Alloc(alloc)
		{
			m_Block = static_cast<type_alloc>(m_Alloc).allocate(1);

			m_Block->UseCount = 1;
			m_Block->Data = ptr;
		}

		ref_ptr(const ref_ptr& other) noexcept :
			m_Alloc(std::allocator_traits<Alloc>::select_on_container_copy_construction(static_cast<Alloc>(other))),
			m_Block(other.m_Block)
		{
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

			m_Block = static_cast<type_alloc>(m_Alloc).allocate(1);
			m_Block->Data = ptr;
			if (ptr == nullptr)
				m_Block->UseCount = 0;
			else
				m_Block->UseCount = 1;
		}

		size_t use_count() const noexcept { return m_Block->UseCount; }

		bool unique() const noexcept { return m_Block->UseCount == 1; }

		operator bool() const noexcept { return m_Block; }

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
				std::allocator_traits<Alloc>::destroy(m_Alloc, m_Block->Data);

				static_cast<type_alloc>(m_Alloc).deallocate(m_Block, 1);
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
		using type_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<control_block>;
		using byte_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<char>;

		control_block* ptr = reinterpret_cast<control_block*>(static_cast<byte_alloc>(alloc).allocate(sizeof(control_block) + sizeof(T)));

		ptr->Data = reinterpret_cast<T*>(ptr + 1);

		std::allocator_traits<Alloc>::construct(alloc, ptr->Data, std::forward<Args>(args)...);

		ref_ptr<T, Alloc> ref(alloc);

		static_cast<type_alloc>(alloc).deallocate(ref.m_Block, 1);
		ref.m_Block = ptr;

		return ref;
	}

	template<typename T, typename ...Args>
	ref_ptr<T> make_ref(Args&& ...args)
	{
		return ref_ptr<T>(new T(std::forward<Args>(args)...));
	}
}