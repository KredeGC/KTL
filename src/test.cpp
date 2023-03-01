#include "shared/test.h"

#include <iostream>

#if defined(_WIN32) // Windows
#define KTL_BREAKPOINT() __debugbreak()
#elif defined(__linux__) // Linux
#include <csignal>
#define KTL_BREAKPOINT() std::raise(SIGTRAP)
#else // Non-supported
#define KTL_BREAKPOINT() throw
#endif

namespace ktl::test
{
	void unit::add_test(const char* name, test_ptr_t func_ptr)
	{
		s_TestNames[s_TestCounter] = name;
		s_TestFunctions[s_TestCounter] = func_ptr;

		s_TestCounter++;
	}

	void unit::run_all_tests()
	{
		size_t count = 0;
		size_t passed = 0;

		for (uint32_t i = 0; i < s_TestCounter; i++)
		{
			if (s_TestFunctions[i])
			{
				count++;
				std::cout << "[" << count << "] " << s_TestNames[i] << " running" << std::endl;
				try
				{
					s_TestFunctions[i]();
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
			else
			{
				std::cout << s_TestNames[i] << std::endl;
			}
		}

		std::cout << passed << " out of " << count << " tests passed" << std::endl;

		if (passed < count)
			KTL_BREAKPOINT();
	}
}