#include "shared/profiler.h"
#include "shared/test.h"

int main(int argc, char* argv[])
{
	ktl::test::unit::run_all_tests();

	ktl::performance::profiler::run_all_benchmarks();

#ifdef _MSC_VER
	system("pause");
#endif

	return 0;
}