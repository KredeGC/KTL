#pragma once

#include <string>

#define KTL_ASSERT(x) if (!(x)) { throw std::string("\"") + #x + "\" in " + std::string(__FILE__) + ":" + std::to_string(__LINE__); }

#define KTL_ASSERT_FALSE() throw std::string("in ") + std::string(__FILE__) + ":" + std::to_string(__LINE__);