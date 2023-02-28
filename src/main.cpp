#include "shared/profiler.h"
#include "shared/test.h"

#include "ktl/ktl_alloc_fwd.h"
#include "ktl/ktl_container_fwd.h"
#include "ktl/ktl.h"

int main(int argc, char* argv[])
{
	ktl::test::unit::run_all_tests();

	ktl::performance::profiler::run_all_benchmarks();

	return 0;
}