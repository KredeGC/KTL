#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

namespace ktl::performance
{
	class profiler;
}

#define KTL_ADD_PERFORMANCE3(func_ptr, line) void func_ptr(); \
	static int test_##line = (::ktl::performance::profiler::add_benchmark(#func_ptr, func_ptr), 0); \
	void func_ptr()

#define KTL_ADD_PERFORMANCE2(func_ptr, line) KTL_ADD_PERFORMANCE3(func_ptr, line)

#define KTL_ADD_PERFORMANCE(func_ptr) KTL_ADD_PERFORMANCE2(func_ptr, __LINE__)

namespace ktl::performance
{
	template<typename T, size_t Count = 1, typename Alloc>
	void perform_ordered_allocation(Alloc& alloc)
	{
		profiler::pause();

		T* ptrs[Count];

		profiler::resume();

		for (size_t i = 0; i < Count; i++)
			ptrs[i] = std::allocator_traits<Alloc>::allocate(alloc, 1);

		profiler::pause();

		for (size_t i = 1; i <= Count; i++)
			std::allocator_traits<Alloc>::deallocate(alloc, ptrs[Count - i], 1);

		profiler::resume();
	}

	template<typename T, size_t Count = 1, typename Alloc>
	void perform_ordered_deallocation(Alloc& alloc)
	{
		profiler::pause();

		T* ptrs[Count];

		for (size_t i = 0; i < Count; i++)
			ptrs[i] = std::allocator_traits<Alloc>::allocate(alloc, 1);

		profiler::resume();

		for (size_t i = 1; i <= Count; i++)
			std::allocator_traits<Alloc>::deallocate(alloc, ptrs[Count - i], 1);
	}

	template<typename T, size_t Count = 1, typename Alloc>
	void perform_unordered_allocation(Alloc& alloc)
	{
		profiler::pause();

		T* ptrs[Count];

		profiler::resume();

		for (size_t i = 0; i < Count; i++)
			ptrs[i] = std::allocator_traits<Alloc>::allocate(alloc, 1);

		profiler::pause();

		std::shuffle(ptrs, ptrs + Count, random_generator);

		profiler::resume();

		for (size_t i = 0; i < Count; i++)
			std::allocator_traits<Alloc>::deallocate(alloc, ptrs[i], 1);
	}

	template<typename T, size_t Count = 1, typename Alloc>
	void perform_unordered_deallocation(Alloc& alloc)
	{
		profiler::pause();

		T* ptrs[Count];

		for (size_t i = 0; i < Count; i++)
			ptrs[i] = std::allocator_traits<Alloc>::allocate(alloc, 1);

		std::shuffle(ptrs, ptrs + Count, random_generator);

		profiler::resume();

		for (size_t i = 0; i < Count; i++)
			std::allocator_traits<Alloc>::deallocate(alloc, ptrs[i], 1);
	}

	class profiler
	{
	public:
		inline constexpr static size_t RUN_COUNT = 1000;

	private:
		inline static std::chrono::steady_clock::time_point s_PausePoint;
		inline static double* s_Duration;

		inline constexpr static size_t MAX_TESTS = 1024;

		inline static void (*s_TestFunctions[MAX_TESTS])();
		inline static std::string s_TestNames[MAX_TESTS];
		inline static size_t s_TestCounter;

	public:
		static inline void start(double* duration)
		{
			s_PausePoint = std::chrono::steady_clock::now();
			s_Duration = duration;
		}

		static inline void pause()
		{
			auto currentPoint = std::chrono::steady_clock::now();

			auto start = std::chrono::time_point_cast<std::chrono::microseconds>(s_PausePoint).time_since_epoch();
			auto end = std::chrono::time_point_cast<std::chrono::microseconds>(currentPoint).time_since_epoch();

			*s_Duration += (end - start).count();

			s_PausePoint = std::chrono::steady_clock::now();
		}

		static inline void resume()
		{
			auto currentPoint = std::chrono::steady_clock::now();

			s_PausePoint = currentPoint;
		}

		static void add_benchmark(const std::string& name, void (*func_ptr)());
		static void run_all_benchmarks();
	};
}