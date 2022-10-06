#pragma once
#include "stdafx.h"

/* static wrapper of std::mt19937_64 */
void SeedEngine();
int RandomInteger(int maxExclusive);
