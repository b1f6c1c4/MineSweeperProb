#pragma once

#include <limits>
#include <stdexcept>

#ifndef NDEBUG
#define ASSERT(val) do { if (!(val)) throw std::runtime_error("oops"); } while (false)
#else
#define ASSERT(val)
#endif

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define NAN std::numeric_limits<double>::quiet_NaN()
