#pragma once

#include <limits>

#ifdef _DEBUG
#define ASSERT(val) do { if (!(val)) throw; } while (false)
#else
#define ASSERT(val)
#endif

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define NAN std::numeric_limits<double>::quiet_NaN()
