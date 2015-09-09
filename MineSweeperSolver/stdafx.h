#pragma once
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

#ifdef MINESWEEPERSOLVER_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#ifdef _DEBUG
#define ASSERT(val) do { ASSERT(val) if (!(val)) throw; } while (false)
#else
#define ASSERT(val)
#endif
