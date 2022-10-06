#pragma once

#include <chrono>
#include <iostream>
#include <string>

#define KTL_ADD_PERFORMANCE3(func_ptr, line) void func_ptr(); \
	static int test_##line = (::ktl::profiler::add_performance_test(#func_ptr, func_ptr), 0); \
	void func_ptr()

#define KTL_ADD_PERFORMANCE2(func_ptr, line) KTL_ADD_PERFORMANCE3(func_ptr, line)

#define KTL_ADD_PERFORMANCE(func_ptr) KTL_ADD_PERFORMANCE2(func_ptr, __LINE__)

namespace ktl
{
	class profiler
	{
	public:
		inline constexpr static size_t NUM_ALLOCATIONS = 10000;
		inline constexpr static size_t RUN_COUNT = 100;

	private:
		inline constexpr static size_t MAX_TESTS = 1024;

		inline static void (*test_functions[MAX_TESTS])();
		inline static std::string test_names[MAX_TESTS];
		inline static size_t test_counter;

	public:
		static void add_performance_test(const std::string& name, void (*func_ptr)());
		static void run_all_performance_tests();
	};
}