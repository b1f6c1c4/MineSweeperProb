#pragma once
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

#ifdef MINESWEEPERSOLVER_EXPORTS
#define MINESWEEPERSOLVER_API __declspec(dllexport)
#else
#define MINESWEEPERSOLVER_API __declspec(dllimport)
#endif
