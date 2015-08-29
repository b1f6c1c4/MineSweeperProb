#pragma once
#include "stdafx.h"
#include "Solver.h"
#include <vector>

struct
    DLL_API BlockProperty
{
    int Index;
    int X, Y;
    int Degree;
    bool IsOpen;
    bool IsMine;
};

struct GameStatus;

class DLL_API GameMgr
{
public:
    GameMgr(int width, int height, int totalMines);
    ~GameMgr();

    size_t DrainCriterion;

    Solver &GetSolver();
    const Solver &GetSolver() const;

    int GetTotalWidth() const;
    int GetTotalHeight() const;
    int GetTotalMines() const;
    int GetToOpen() const;
    bool GetStarted() const;
    bool GetSucceed() const;
    double GetBits() const;
    double GetAllBits() const;

    const BlockProperty &GetBlockProperty(int x, int y) const;
    const BlockProperty *GetBlockProperties() const;
    double GetBlockProbability(int x, int y) const;
    BlockStatus GetInferredStatus(int x, int y) const;

    const Block *GetBestBlocks() const;
    int GetBestBlockCount() const;
    const Block *GetPreferredBlocks() const;
    int GetPreferredBlockCount() const;

    void OpenBlock(int x, int y);

    void Solve(SolvingState maxDepth, bool shortcut);

    void OpenOptimalBlocks();

    bool SemiAutomaticStep(SolvingState maxDepth);
    bool SemiAutomatic(SolvingState maxDepth);
    void AutomaticStep(SolvingState maxDepth);
    void Automatic();

    void EnableDrainer();

    friend class Drainer;
private:
    int m_TotalWidth, m_TotalHeight, m_TotalMines;
    bool m_Settled, m_Started, m_Succeed;
    std::vector<BlockProperty> m_Blocks;
    std::vector<BlockSet> m_BlocksR;
    int m_ToOpen;
    Solver m_Solver;
    double m_AllBits;
    std::vector<Block> m_Best, m_Preferred;
    Drainer *m_Drainer;

    int GetIndex(int x, int y) const;

    void SettleMines(int initID);
    void OpenBlock(int id);
};
