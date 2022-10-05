#pragma once

#include <cstddef>
#include <string>

#define KTL_ADD_TEST3(func_ptr, line) void func_ptr(); \
	static int test_##line = (::ktl::test::add_test(#func_ptr, func_ptr), 0); \
	void func_ptr()

#define KTL_ADD_TEST2(func_ptr, line) KTL_ADD_TEST3(func_ptr, line)

#define KTL_ADD_TEST(func_ptr) KTL_ADD_TEST2(func_ptr, __LINE__)

#define KTL_ADD_HEADER3(name, line) static int test_##line = (::ktl::test::add_test(name, nullptr), 0)

#define KTL_ADD_HEADER2(name, line) KTL_ADD_HEADER3(name, line)

#define KTL_ADD_HEADER() KTL_ADD_HEADER2(__FILE__, __LINE__)

#define KTL_ADD_HEADER_NAME(name) KTL_ADD_HEADER2(name, __LINE__)

namespace ktl
{
	class test
	{
	private:
		inline constexpr static size_t MAX_TESTS = 1024;

		inline static void (*test_functions[MAX_TESTS])();
		inline static std::string test_names[MAX_TESTS];
		inline static size_t test_counter;

	public:
		static void add_test(const std::string& name, void (*func_ptr)());

		static void run_all_tests();
	};
}