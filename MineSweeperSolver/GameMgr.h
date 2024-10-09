#pragma once
#include "stdafx.h"
#include "Solver.h"
#include "Drainer.h"
#include <optional>
#include <vector>
#include <memory>
#include "Strategies.h"

struct
    BlockProperty
{
    int Index;
    int X, Y;
    int Degree;
    bool IsOpen;
    bool IsMine;
    bool IsRelevant2;
};

template <typename T>
struct EmptyCopyable : std::unique_ptr<T>
{
    EmptyCopyable() = default;
    EmptyCopyable(std::unique_ptr<T> &&other) : std::unique_ptr<T>{ std::move(other) } { }
    EmptyCopyable(const EmptyCopyable<T> &other) : std::unique_ptr<T>{} { }
    EmptyCopyable(EmptyCopyable<T> &&other) noexcept = default;
    auto &operator=(const EmptyCopyable<T> &other) noexcept { return *this; }
    EmptyCopyable<T> &operator=(EmptyCopyable<T> &&other) = default;
};

/* Bookkeeping the gaming process, store mine locations, report degree information to solvers.
 * Use information from BasicSolver/Solver/Drainer to open corresponding blocks.
 */
class
    GameMgr
{
public:
    GameMgr(int width, int height, int totalMines, bool isSNR, Strategy strategy, bool allowWrongGuess = false);
    GameMgr(int width, int height, int totalMines, Strategy strategy);
    GameMgr(std::istream &sr, Strategy strategy);
    GameMgr(const GameMgr &) = default;
    GameMgr(GameMgr &&) noexcept = default;
    GameMgr &operator=(const GameMgr &) = default;
    GameMgr &operator=(GameMgr &&) noexcept = default;

    Strategy BasicStrategy;

    Solver &GetSolver();
    [[nodiscard]] const Solver &GetSolver() const;
    [[nodiscard]] const Drainer *GetDrainer() const;

    [[nodiscard]] int GetTotalWidth() const;
    [[nodiscard]] int GetTotalHeight() const;
    [[nodiscard]] int GetTotalMines() const;
    [[nodiscard]] int GetToOpen() const;
    [[nodiscard]] int GetWrongGuesses() const;
    [[nodiscard]] bool GetSettled() const;
    [[nodiscard]] bool GetStarted() const;
    [[nodiscard]] bool GetSucceed() const;
    [[nodiscard]] double GetBits() const;
    [[nodiscard]] double GetAllBits() const;
    [[nodiscard]] int GetLastProbe() const;
    [[nodiscard]] double GetMinProbability() const;
    [[nodiscard]] double GetMaxProbability() const;

    [[nodiscard]] const BlockProperty &GetBlockProperty(int x, int y) const;
    const BlockProperty &SetBlockDegree(int x, int y, int degree);
    const BlockProperty &SetBlockDegree(int id, int degree);
    const BlockProperty &SetBlockMine(int x, int y, bool mined);
    [[nodiscard]] const BlockProperty *GetBlockProperties() const;
    [[nodiscard]] double GetBlockProbability(int x, int y) const;
    [[nodiscard]] BlockStatus GetInferredStatus(int x, int y) const;

    [[nodiscard]] const Block *GetBestBlocks() const;
    [[nodiscard]] size_t GetBestBlockCount() const;
    [[nodiscard]] const BlockSet &GetBestBlockList() const;
    [[nodiscard]] const Block *GetPreferredBlocks() const;
    [[nodiscard]] size_t GetPreferredBlockCount() const;
    [[nodiscard]] const BlockSet &GetPreferredBlockList() const;
    [[nodiscard]] const std::vector<double> &GetBestProbabilityList() const;

    bool OpenBlock(int x, int y);

    void Solve(SolvingState maxDepth, bool shortcut);

    void OpenOptimalBlocks();

    bool SemiAutomaticStep(SolvingState maxDepth, bool single, bool nearest = false);
    bool SemiAutomatic(SolvingState maxDepth);
    void AutomaticStep(SolvingState maxDepth);
    void Automatic(bool drain = true);

    void EnableDrainer(bool drain);
    [[nodiscard]] size_t GetDrainerSteps() const;
    [[nodiscard]] bool MakeDrainerProgress();

    void Save(std::ostream &sw) const;

    friend class Drainer;
private:
    bool m_IsExternal;
    bool m_AllowWrongGuess;
    int m_TotalWidth, m_TotalHeight, m_TotalMines;
    bool m_IsSNR;
    bool m_Settled, m_Started, m_Succeed;
    std::vector<BlockProperty> m_Blocks;
    std::vector<BlockSet> m_BlocksR; // each block's neighbor
    int m_ToOpen, m_WrongGuesses;
    std::optional<Solver> m_Solver;
    double m_AllBits;
    BlockSet m_Best, m_Preferred;
    EmptyCopyable<Drainer> m_Drainer;
    int m_LastProbe;

    [[nodiscard]] int GetIndex(int x, int y) const;

    void GenerateBlocksR();
    void SettleMines(int initID);
    void OpenBlockImpl(int id);
    void UpdateRelevant2Info(int id);

    [[nodiscard]] int FrontierDist(Block blk) const;
};

template <typename F>
std::enable_if_t<std::is_same_v<std::invoke_result_t<F, Block>, int>, void>
Largest(BlockSet &bests, const F &fun)
{
    if (bests.size() <= 1)
        return;
    BlockSet newBests;
    newBests.push_back(bests.front());
    auto bestVal = fun(bests.front());
    for (auto i = 1; i < bests.size(); ++i)
    {
        auto p = fun(bests[i]);
        if (bestVal < p)
        {
            bestVal = p;
            newBests.clear();
        }
        if (bestVal <= p)
            newBests.push_back(bests[i]);
    }
    newBests.swap(bests);
}

template <typename F>
std::enable_if_t<std::is_same_v<std::invoke_result_t<F, Block>, double>, void>
Largest(BlockSet &bests, const F &fun)
{
    if (bests.size() <= 1)
        return;
    BlockSet newBests;
    newBests.push_back(bests.front());
    auto bestVal = fun(bests.front());
    for (auto i = 1; i < bests.size(); ++i)
    {
        auto p = fun(bests[i]);
        if (bestVal < p)
        {
            bestVal = p;
            newBests.clear();
        }
        if (bestVal - std::abs(bestVal) * 1E-8 <= p)
            newBests.push_back(bests[i]);
    }
    newBests.swap(bests);
}
