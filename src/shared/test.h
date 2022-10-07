#pragma once

#include <cstddef>
#include <string>

#define KTL_ADD_TEST3(func_ptr, line) void func_ptr(); \
	static int test_##line = (::ktl::test::unit::add_test(#func_ptr, func_ptr), 0); \
	void func_ptr()

#define KTL_ADD_TEST2(func_ptr, line) KTL_ADD_TEST3(func_ptr, line)

#define KTL_ADD_TEST(func_ptr) KTL_ADD_TEST2(func_ptr, __LINE__)

#define KTL_ADD_HEADER3(name, line) static int test_##line = (::ktl::test::unit::add_test(name, nullptr), 0)

#define KTL_ADD_HEADER2(name, line) KTL_ADD_HEADER3(name, line)

#define KTL_ADD_HEADER() KTL_ADD_HEADER2(__FILE__, __LINE__)

namespace ktl::test
{
	class unit
	{
	private:
		inline constexpr static size_t MAX_TESTS = 1024;

		inline static void (*s_TestFunctions[MAX_TESTS])();
		inline static std::string s_TestNames[MAX_TESTS];
		inline static size_t s_TestCounter;

	public:
		static void add_test(const std::string& name, void (*func_ptr)());

		static void run_all_tests();
	};
}