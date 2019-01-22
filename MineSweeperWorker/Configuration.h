#pragma once
#include "stdafx.h"
#include "../MineSweeperSolver/Strategies.h"

struct Configuration : Strategy
{
    int Width;
    int Height;

    bool IsTotalMine;
    int TotalMines;
    double Probability;

    bool Slack;
};
