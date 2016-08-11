#pragma once
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

#ifdef _DEBUG
#define ASSERT(val) do { if (!(val)) throw; } while (false)
#else
#define ASSERT(val)
#endif
