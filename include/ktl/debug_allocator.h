#pragma once

#include <memory>
#include <type_traits>

#include <iostream>

namespace ktl
{
	template<typename T>
	class pre_allocator
	{
	public:
		using value_type = T;
		using is_always_equal = std::true_type;

		template<typename U>
		struct rebind
		{
			using other = pre_allocator<U>;
		};

		pre_allocator() noexcept = default;

		template<typename U>
		pre_allocator(const pre_allocator<U>&) noexcept {}

		T* allocate(size_t n)
		{
			std::cout << "Allocated: " << n << " objects" << std::endl;

			return static_cast<T*>(::operator new(sizeof(T) * n));
		}

		void deallocate(T* p, size_t n)
		{
			std::cout << "Deallocated: " << n << " objects" << std::endl;

			::operator delete(p);
		}
	};

	template<typename T, typename U>
	bool operator==(const pre_allocator<T>&, const pre_allocator<U>&) noexcept
	{
		return true;
	}

	template<typename T, typename U>
	bool operator!=(const pre_allocator<T>&, const pre_allocator<U>&) noexcept
	{
		return false;
	}
}