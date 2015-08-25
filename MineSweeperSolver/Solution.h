#pragma once
#include "stdafx.h"
#include <vector>
#include "BigInteger.h"

class Solution
{
public:
    explicit Solution(std::vector<int> &&dist);
    ~Solution();

    friend class Solver;
private:
    std::vector<int> Dist;
    BigInteger States;
    double Ratio;
};
