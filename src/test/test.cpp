#include "test.h"

#include <iostream>

#if defined(_WIN32) // Windows
#define KTL_BREAKPOINT() __debugbreak()
#elif defined(__linux__) // Linux
#include <csignal>
#define KTL_BREAKPOINT() std::raise(SIGTRAP)
#else // Non-supported
#define KTL_BREAKPOINT() throw
#endif

namespace ktl
{
	void test::add_test(const std::string& name, void (*func_ptr)())
	{
		test_names[test_counter] = name;
		test_functions[test_counter] = func_ptr;

		test_counter++;
	}

	void test::run_all_tests()
	{
		size_t passed = 0;

		for (uint32_t i = 0; i < test_counter; i++)
		{
			std::cout << "[" << (i + 1) << "] " << test_names[i] << " running" << std::endl;
			try
			{
				test_functions[i]();
				std::cout << "  ->passed" << std::endl;
				passed++;
			}
			catch (const char* e)
			{
				std::cout << "  ->failed assertion: " << e << std::endl;
			}
			catch (const std::string& e)
			{
				std::cout << "  ->failed assertion: " << e << std::endl;
			}
			catch (const std::exception& e)
			{
				std::cout << "  ->exception encountered: " << e.what() << std::endl;
			}
			catch (...)
			{
				std::cout << "  ->fatal error" << std::endl;
			}
		}

		std::cout << passed << " out of " << test_counter << " tests passed" << std::endl;

		if (passed < test_counter)
			KTL_BREAKPOINT();

#ifdef _MSC_VER
		system("pause");
#endif
	}
}

int main(int argc, char* argv[])
{
	ktl::test::run_all_tests();

	return 0;
}