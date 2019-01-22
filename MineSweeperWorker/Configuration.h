#pragma once
#include "stdafx.h"
#include "../MineSweeperSolver/Strategies.h"

struct Configuration : Strategy
{
    int Width;
    int Height;

    int TotalMines;
};

struct RawConfiguration
{
    int Width;
    int Height;

    int TotalMines;

    int InitialPosition;

    LogicMethod Logic;

    bool HeuristicEnabled;
    HeuristicMethod *DecisionTree;
    int DecisionTreeLen;

    bool ExhaustEnabled;
    int ExhaustCriterion;
};
