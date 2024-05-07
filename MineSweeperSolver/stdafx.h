#pragma once

#include <cmath>
#include <cstddef>
#include <cstring>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#ifndef NDEBUG
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define ASSERT(val) do { \
    if (!(val)) \
        throw std::runtime_error("assert failure at " __FILE__ " line " STRINGIZE(__LINE__)); \
    } while (false)
#else
#define ASSERT(val)
#endif

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
