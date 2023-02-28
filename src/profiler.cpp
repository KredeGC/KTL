#include "shared/profiler.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

namespace ktl::performance
{
	void profiler::add_benchmark(const char* name, FuncPtr func_ptr)
	{
		s_ProfileNames[s_ProfileCounter] = name;
		s_ProfileFunctions[s_ProfileCounter] = func_ptr;

		s_ProfileCounter++;
	}

	void profiler::run_all_benchmarks()
	{
		std::vector<double> run_times;
		run_times.reserve(RUN_COUNT);

		for (uint32_t i = 0; i < s_ProfileCounter; i++)
		{
			std::cout << "[" << (i + 1) << "] " << s_ProfileNames[i] << " running " << RUN_COUNT << " runs" << std::endl;
			double minDuration = std::numeric_limits<double>::infinity();
			double maxDuration = -std::numeric_limits<double>::infinity();
			double avgDuration = 0.0;
			for (size_t c = 0; c < RUN_COUNT; c++)
			{
				double duration = 0.0;
				start(&duration);

				s_ProfileFunctions[i]();

				pause();

				if (duration < minDuration)
					minDuration = duration;

				if (duration > maxDuration)
					maxDuration = duration;

				avgDuration += duration;

				run_times.push_back(duration);
			}

			std::sort(run_times.begin(), run_times.end());

			avgDuration /= RUN_COUNT;
			std::cout << "  ->Min:    " << minDuration << "us" << std::endl;
			std::cout << "  ->Max:    " << maxDuration << "us" << std::endl;
			std::cout << "  ->Mean:   " << avgDuration << "us" << std::endl;
			std::cout << "  ->Median: " << run_times[RUN_COUNT / 2 - 1] << "us" << std::endl;

			run_times.clear();
		}
	}
}