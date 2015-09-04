#pragma once
#include "stdafx.h"
#include "Solver.h"
#include <vector>
#include <functional>

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

class
    DLL_API GameMgr
{
public:
    GameMgr(int width, int height, int totalMines);
    explicit GameMgr(std::istream &sr);
    ~GameMgr();

    size_t DrainCriterion;

    Solver &GetSolver();
    const Solver &GetSolver() const;
    const Drainer *GetDrainer() const;

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

    void Save(std::ostream &sw) const;

    friend class Drainer;
private:
    int m_TotalWidth, m_TotalHeight, m_TotalMines;
    bool m_Settled, m_Started, m_Succeed;
    std::vector<BlockProperty> m_Blocks;
    std::vector<BlockSet> m_BlocksR;
    int m_ToOpen;
    Solver *m_Solver;
    double m_AllBits;
    BlockSet m_Best, m_Preferred;
    Drainer *m_Drainer;

    int GetIndex(int x, int y) const;

    void SettleMines(int initID);
    void OpenBlock(int id);

    int FrontierDist(Block blk) const;
};

void Largest(BlockSet &bests, std::function<int(Block)> fun);
void Largest(BlockSet &bests, std::function<double(Block)> fun);
