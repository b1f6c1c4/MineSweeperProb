#pragma once
#include "BigInteger.h"

extern "C" DLL_API void CacheBinomials(int n, int m);
DLL_API BigInteger Binomial(int n, int m);
