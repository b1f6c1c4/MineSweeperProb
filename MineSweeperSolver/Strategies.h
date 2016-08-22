#pragma once
#include "stdafx.h"
#include <vector>

enum class LogicMethod
{
    Passive = 0x00,
    Single = 0x01,
    SingleExtended = 0x02,
    Double = 0x03,
    DoubleExtended = 0x04,
    Full = 0x05
};

enum class HeuristicMethod
{
    None = 0x00,
    MinMineProb = 0x01,
    MaxZeroProb = 0x02,
    MaxZerosProb = 0x03,
    MaxZerosExp = 0x04,
    MaxQuantityExp = 0x05,
    MinFrontierDist = 0x06,
    MaxUpperBound = 0x07
};

struct Strategy
{
    bool InitialPositionSpecified;
    int Index;

    LogicMethod Logic;

    bool HeuristicEnabled;
    std::vector<HeuristicMethod> DecisionTree;

    bool ExhaustEnabled, PruningEnabled;
    int ExhaustCriterion, PruningCriterion;
    std::vector<HeuristicMethod> PruningDecisionTree;
};
