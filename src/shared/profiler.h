#pragma once

#include "random.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

namespace ktl::performance
{
	class profiler;
}

#define KTL_ADD_BENCHMARK3(func_ptr, line) void func_ptr(); \
	struct profile_struct##line { profile_struct##line() { ::ktl::performance::profiler::add_benchmark(#func_ptr, func_ptr); } }; \
	static profile_struct##line profile_##line; \
	void func_ptr()

#define KTL_ADD_BENCHMARK2(func_ptr, line) KTL_ADD_BENCHMARK3(func_ptr, line)

#define KTL_ADD_BENCHMARK(func_ptr) KTL_ADD_BENCHMARK2(func_ptr, __LINE__)

namespace ktl::performance
{
	class profiler
	{
	public:
		typedef void (*FuncPtr)();

		inline constexpr static size_t RUN_COUNT = 1000;
		inline constexpr static size_t MAX_BENCHMARKS = 128;

	private:
		inline static std::chrono::steady_clock::time_point s_PausePoint;
		inline static double* s_Duration;
		inline static bool s_Paused;

		inline static FuncPtr s_ProfileFunctions[MAX_BENCHMARKS];
		inline static const char* s_ProfileNames[MAX_BENCHMARKS];
		inline static size_t s_ProfileCounter;

	public:
		static inline void start(double* duration)
		{
			s_PausePoint = std::chrono::steady_clock::now();
			s_Duration = duration;
			s_Paused = false;
		}

		static inline void pause()
		{
			auto currentPoint = std::chrono::steady_clock::now();

			if (!s_Paused)
			{
				s_Paused = true;

				auto start = std::chrono::time_point_cast<std::chrono::microseconds>(s_PausePoint).time_since_epoch();
				auto end = std::chrono::time_point_cast<std::chrono::microseconds>(currentPoint).time_since_epoch();

				*s_Duration += (end - start).count();

				s_PausePoint = std::chrono::steady_clock::now();
			}
		}

		static inline void resume()
		{
			s_PausePoint = std::chrono::steady_clock::now();
			s_Paused = false;
		}

		static void add_benchmark(const char* name, FuncPtr func_ptr);
		static void run_all_benchmarks();
	};

	template<typename T, size_t Count = 1, typename Alloc>
	void perform_allocation(Alloc& alloc)
	{
		profiler::pause();

		T** ptrs = new T *[Count];

		profiler::resume();

		for (size_t i = 0; i < Count; i++)
			ptrs[i] = std::allocator_traits<Alloc>::allocate(alloc, 1);

		profiler::pause();

		for (size_t i = 1; i <= Count; i++)
			std::allocator_traits<Alloc>::deallocate(alloc, ptrs[Count - i], 1);

		delete[] ptrs;
	}

	template<typename T, size_t Count = 1, typename Alloc>
	void perform_ordered_deallocation(Alloc& alloc)
	{
		profiler::pause();

		T** ptrs = new T *[Count];

		for (size_t i = 0; i < Count; i++)
			ptrs[i] = std::allocator_traits<Alloc>::allocate(alloc, 1);

		profiler::resume();

		for (size_t i = 1; i <= Count; i++)
			std::allocator_traits<Alloc>::deallocate(alloc, ptrs[Count - i], 1);

		profiler::pause();

		delete[] ptrs;
	}

	template<typename T, size_t Count = 1, typename Alloc>
	void perform_unordered_allocation(Alloc& alloc)
	{
		profiler::pause();

		T** ptrs = new T*[Count];
		T** random_ptrs = new T*[Count];

		for (size_t i = 0; i < Count; i++)
			ptrs[i] = std::allocator_traits<Alloc>::allocate(alloc, 1);

		std::memcpy(random_ptrs, ptrs, sizeof(T*) * Count);
		std::shuffle(random_ptrs, random_ptrs + Count, random_generator);

		for (size_t i = 0; i < Count / 2; i++)
			std::allocator_traits<Alloc>::deallocate(alloc, random_ptrs[i], 1);

		profiler::resume();

		for (size_t i = 0; i < Count / 2; i++)
			random_ptrs[i] = std::allocator_traits<Alloc>::allocate(alloc, 1);

		profiler::pause();

		for (size_t i = 0; i < Count; i++)
			std::allocator_traits<Alloc>::deallocate(alloc, random_ptrs[i], 1);

		delete[] ptrs;
		delete[] random_ptrs;
	}

	template<typename T, size_t Count = 1, typename Alloc>
	void perform_unordered_deallocation(Alloc& alloc)
	{
		profiler::pause();

		T** ptrs = new T*[Count];

		for (size_t i = 0; i < Count; i++)
			ptrs[i] = std::allocator_traits<Alloc>::allocate(alloc, 1);

		std::shuffle(ptrs, ptrs + Count, random_generator);

		profiler::resume();

		for (size_t i = 0; i < Count; i++)
			std::allocator_traits<Alloc>::deallocate(alloc, ptrs[i], 1);

		profiler::pause();

		delete[] ptrs;
	}
}