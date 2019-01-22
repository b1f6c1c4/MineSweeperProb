#pragma once
#include "stdafx.h"
#include <random>
#include <algorithm>
#include <functional>

template <class T = std::mt19937, size_t N = T::state_size>
auto ProperlySeededRandomEngine() -> typename std::enable_if<!!N, T>::type
{
    typename T::result_type random_data[N];
    std::random_device source;
    std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
    std::seed_seq seeds(std::begin(random_data), std::end(random_data));
    T seededEngine(seeds);
    return seededEngine;
}

int RandomInteger(int maxExclusive);
