#pragma once
#include "stdafx.h"
#include "Solver.h"
#include <vector>
#include <functional>
#include "Strategies.h"

struct
    DLL_API BlockProperty
{
    int Index;
    int X, Y;
    int Degree;
    bool IsOpen;
    bool IsMine;
};

class DLL_API Drainer;

class
    DLL_API GameMgr
{
public:
    GameMgr(int width, int height, int totalMines, bool allowWrongGuess = false);
    explicit GameMgr(std::istream &sr);
    ~GameMgr();

    Strategy BasicStrategy;

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
    size_t GetBestBlockCount() const;
    const Block *GetPreferredBlocks() const;
    size_t GetPreferredBlockCount() const;

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
    bool m_AllowWrongGuess;
    int m_TotalWidth, m_TotalHeight, m_TotalMines;
    bool m_Settled, m_Started, m_Succeed;
    std::vector<BlockProperty> m_Blocks;
    std::vector<BlockSet> m_BlocksR;
    int m_ToOpen, m_WrongGuesses;
    Solver *m_Solver;
    double m_AllBits;
    BlockSet m_Best, m_Preferred;
    Drainer *m_Drainer;

    int GetIndex(int x, int y) const;

    void SettleMines(int initID);
    void OpenBlock(int id);

    int FrontierDist(Block blk) const;
};

DLL_API void Largest(BlockSet &bests, std::function<int(Block)> fun);
DLL_API void Largest(BlockSet &bests, std::function<double(Block)> fun);
