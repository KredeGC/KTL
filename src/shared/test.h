#pragma once

#include <cstddef>
#include <string>

#define KTL_ADD_TEST3(func_ptr, line) void func_ptr(); \
	struct test_struct_##func_ptr##_##line { test_struct_##func_ptr##_##line() { ::ktl::test::unit::add_test(#func_ptr, func_ptr); } }; \
	static test_struct_##func_ptr##_##line test_var_##func_ptr##_##line; \
	void func_ptr()

#define KTL_ADD_TEST2(func_ptr, line) KTL_ADD_TEST3(func_ptr, line)

#define KTL_ADD_TEST(func_ptr) KTL_ADD_TEST2(func_ptr, __LINE__)

namespace ktl::test
{
	class unit
	{
	public:
		typedef void (*FuncPtr)();

		inline constexpr static size_t MAX_TESTS = 256;

	private:
		inline static FuncPtr s_TestFunctions[MAX_TESTS];
		inline static const char* s_TestNames[MAX_TESTS];
		inline static size_t s_TestCounter;

	public:
		static void add_test(const char* name, FuncPtr func_ptr);

		static void run_all_tests();
	};
}