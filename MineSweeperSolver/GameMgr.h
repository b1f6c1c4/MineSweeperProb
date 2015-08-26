#pragma once
#include "stdafx.h"
#include "Solver.h"
#include <vector>

struct DLL_API BlockProperty
{
    int Index;
    int X, Y;
    int Degree;
    bool IsOpen;
    bool IsMine;
};

struct BlockRelationship
{
    BlockSet Self;
    BlockSet Surrounding;
};

struct GameStatus;

class DLL_API GameMgr
{
public:
    GameMgr(int width, int height, int totalMines);

    Solver &GetSolver();
    const Solver &GetSolver() const;

    int GetTotalWidth() const;
    int GetTotalHeight() const;
    int GetTotalMines() const;
    int GetToOpen() const;
    bool IsStarted() const;
    bool IsSucceed() const;
    double GetBits() const;
    double GetAllBits() const;

    const BlockProperty &GetBlockProperty(int x, int y) const;
    const BlockProperty *GetBlockProperties() const;
    double GetBlockProbability(int x, int y) const;
    BlockStatus GetBlockStatus(int x, int y) const;

    void OpenBlock(int x, int y);

    bool SemiAutomaticStep(bool withProb);
    bool SemiAutomatic(bool withProb);
    void AutomaticStep();
    void Automatic();
private:
    int m_TotalWidth, m_TotalHeight, m_TotalMines;
    bool m_Settled, m_Started, m_Succeed;
    std::vector<BlockProperty> m_Blocks;
    std::vector<BlockRelationship> m_BlocksR;
    int m_ToOpen;
    Solver m_Solver;
    double m_AllBits;

    int GetIndex(int x, int y) const;

    void SettleMines(int initID);
    void OpenBlock(int id);
};
