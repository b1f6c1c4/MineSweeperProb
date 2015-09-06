#pragma once
#include <vector>

enum class LogicMethod
{
    None,
    Single,
    Double,
    Full
};

enum class HeuristicMethod
{
    MinMineProb,
    MaxZeroProb,
    MaxZerosProb,
    MaxZerosExp,
    MaxQuantityExp,
    MinFrontierDist
};

struct Strategy
{
    bool InitialPositionSpecified;
    int X, Y;

    LogicMethod Logic;
    
    bool HeuristicEnabled;
    std::vector<HeuristicMethod> DecisionTree;

    bool ExhaustEnabled, PruningEnabled;
    int ExhaustCriterion, PruningCriterion;
    std::vector<HeuristicMethod> PruningDecisionTree;
};
