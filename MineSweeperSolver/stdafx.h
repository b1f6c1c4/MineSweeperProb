#pragma once

#include <cmath>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>

#ifdef _DEBUG
#define ASSERT(val) do { if (!(val)) throw; } while (false)
#else
#define ASSERT(val)
#endif

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define THROW throw std::runtime_error{ std::string{ "Exception in " } + __FILE__ + ':' + std::to_string(__LINE__) + ':' + __func__ }
