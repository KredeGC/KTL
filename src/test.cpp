#include "test/shared/profiler.h"
#include "test/shared/test.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <vector>

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
	void profiler::add_performance_test(const std::string& name, void (*func_ptr)())
	{
		test_names[test_counter] = name;
		test_functions[test_counter] = func_ptr;

		test_counter++;
	}

	void test::add_test(const std::string& name, void (*func_ptr)())
	{
		test_names[test_counter] = name;
		test_functions[test_counter] = func_ptr;

		test_counter++;
	}

	void test::run_all_tests()
	{
		size_t count = 0;
		size_t passed = 0;

		for (uint32_t i = 0; i < test_counter; i++)
		{
			if (test_functions[i])
			{
				count++;
				std::cout << "[" << count << "] " << test_names[i] << " running" << std::endl;
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
			else
			{
				std::cout << test_names[i] << std::endl;
			}
		}

		std::cout << passed << " out of " << count << " tests passed" << std::endl;

		if (passed < count)
			KTL_BREAKPOINT();
	}

	void profiler::run_all_performance_tests()
	{
		std::vector<double> run_times;
		run_times.reserve(100);

		for (uint32_t i = 0; i < test_counter; i++)
		{
			std::cout << "[" << (i + 1) << "] " << test_names[i] << " running " << profiler::RUN_COUNT << " runs" << std::endl;
			double minDuration = std::numeric_limits<double>::infinity();
			double maxDuration = -std::numeric_limits<double>::infinity();
			double avgDuration = 0.0;
			for (size_t c = 0; c < profiler::RUN_COUNT; c++)
			{
				auto startPoint = std::chrono::steady_clock::now();

				test_functions[i]();

				auto endPoint = std::chrono::steady_clock::now();

				auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startPoint).time_since_epoch();
				auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch();

				auto diff = end - start;

				double duration = float(diff.count());

				if (duration < minDuration)
					minDuration = duration;

				if (duration > maxDuration)
					maxDuration = duration;

				avgDuration += duration;

				run_times.push_back(duration);
			}

			std::sort(run_times.begin(), run_times.end());

			avgDuration /= profiler::RUN_COUNT;
			std::cout << "  ->Min: " << minDuration << "us" << std::endl;
			std::cout << "  ->Max: " << maxDuration << "us" << std::endl;
			std::cout << "  ->Mean: " << avgDuration << "us" << std::endl;
			std::cout << "  ->Median: " << run_times[49] << "us" << std::endl;

			run_times.clear();
		}
	}
}

int main(int argc, char* argv[])
{
	ktl::test::run_all_tests();

#ifndef _DEBUG
	ktl::profiler::run_all_performance_tests();
#endif

#ifdef _MSC_VER
	system("pause");
#endif

	return 0;
}