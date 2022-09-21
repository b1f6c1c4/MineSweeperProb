#pragma once

#include <cmath>
#include <cstddef>
#include <cstring>
#include <iostream>

#ifdef _DEBUG
#define ASSERT(val) do { if (!(val)) throw; } while (false)
#else
#define ASSERT(val)
#endif

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
