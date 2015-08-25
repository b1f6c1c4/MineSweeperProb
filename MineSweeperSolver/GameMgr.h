#pragma once
#include "stdafx.h"
#include "Solver.h"
#include <vector>

struct DLL_API BlockProperty
{
    int Index;
    int X, Y;
    BlockSet Surrounding;
    int Degree;
    bool IsOpen;
    bool IsMine;
};

class DLL_API GameMgr
{
public:
    GameMgr(int width, int height, int totalMines);

    const BlockProperty &GetBlockProperty(int x, int y) const;
    double GetBlockProbability(int x, int y) const;
    BlockStatus GetBlockStatus(int x, int y) const;

    void OpenBlock(int x, int y);
    void OpenBlock(int id);
private:
    int m_TotalWidth, m_TotalHeight, m_TotalMines;
    bool m_Settled, m_Started, m_Succeed;
    std::vector<BlockProperty> m_Blocks;
    int m_ToOpen;
    Solver m_Solver;

    int GetIndex(int x, int y) const;

    void SettleMines(int initID);
};
