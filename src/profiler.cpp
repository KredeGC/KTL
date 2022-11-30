#include "shared/profiler.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

namespace ktl::performance
{
	void profiler::add_benchmark(const std::string& name, void (*func_ptr)())
	{
		s_ProfileNames[s_ProfileCounter] = name;
		s_ProfileFunctions[s_ProfileCounter] = func_ptr;

		s_ProfileCounter++;
	}

	void profiler::run_all_benchmarks()
	{
		std::vector<double> run_times;
		run_times.reserve(100);

		for (uint32_t i = 0; i < s_ProfileCounter; i++)
		{
			std::cout << "[" << (i + 1) << "] " << s_ProfileNames[i] << " running " << profiler::RUN_COUNT << " runs" << std::endl;
			double minDuration = std::numeric_limits<double>::infinity();
			double maxDuration = -std::numeric_limits<double>::infinity();
			double avgDuration = 0.0;
			for (size_t c = 0; c < profiler::RUN_COUNT; c++)
			{
				double duration = 0.0;
				profiler::start(&duration);

				s_ProfileFunctions[i]();

				profiler::pause();

				if (duration < minDuration)
					minDuration = duration;

				if (duration > maxDuration)
					maxDuration = duration;

				avgDuration += duration;

				run_times.push_back(duration);
			}

			std::sort(run_times.begin(), run_times.end());

			avgDuration /= profiler::RUN_COUNT;
			std::cout << "  ->Min:    " << minDuration << "us" << std::endl;
			std::cout << "  ->Max:    " << maxDuration << "us" << std::endl;
			std::cout << "  ->Mean:   " << avgDuration << "us" << std::endl;
			std::cout << "  ->Median: " << run_times[profiler::RUN_COUNT / 2 - 1] << "us" << std::endl;

			run_times.clear();
		}
	}
}