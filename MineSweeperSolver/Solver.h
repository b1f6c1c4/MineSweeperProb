#pragma once
#include "stdafx.h"
#include "BasicSolver.h"
#include <map>
#include <functional>

class DistCondQParameters;

/* Compute some heuristic information about blocks that helps break the tie when can't decide next move.
 *
 * These pieces of information are obtained by intersecting a block's neighbor
 * against a certain solved probability model to predict the degree of a block.
 */
class
    Solver : public BasicSolver
{
public:
    explicit Solver(size_t count);
    Solver(size_t count, int mines);
    Solver(const Solver &other);
    ~Solver() override;

    bool Solve(SolvingState maxDepth, bool shortcut) override;

    [[nodiscard]] const DistCondQParameters &GetDistInfo(const BlockSet &set, Block blk, int &min);

    [[nodiscard]] double ZeroCondQ(const BlockSet &set, Block blk);
    [[nodiscard]] double ZerosCondQ(const BlockSet &set, Block blk);
    [[nodiscard]] double ZerosECondQ(const BlockSet &set, Block blk);
    [[nodiscard]] double UpperBoundCondQ(const BlockSet &set, Block blk);
    [[nodiscard]] const std::vector<double> &DistributionCondQ(const BlockSet &set, Block blk, int &min);
    [[nodiscard]] double QuantityCondQ(const BlockSet &set, Block blk);

    friend class Drainer;
private:
    std::multimap<size_t, DistCondQParameters *> m_DistCondQCache;

    std::vector<double> m_DicT_Temp, m_Cases_Temp;
    std::vector<double> m_Add_Temp;


    static void Merge(const std::vector<double> &from, std::vector<double> &to);
    void Add(std::vector<double> &from, const std::vector<double> &cases);

    /* Prepare for distribution computation
     *
     * IN set: neighbor of <blk>
     * IN blk: which block to consider
     * OUT min: number of confirmed mines in <set>
     */
    [[nodiscard]] DistCondQParameters PackParameters(const BlockSet &set, Block blk, int &min) const;

    /* Compute <par>.m_Halves */
    void GetHalves(DistCondQParameters &par) const;
    /* Actually compute the distribution */
    void EnumerateSolutions(DistCondQParameters &par) const;

    [[nodiscard]] DistCondQParameters *TryGetCache(DistCondQParameters &&par, std::function<bool(const DistCondQParameters &)> pre);

    /* <par>.m_Result[0] = P(blk.degree == 0) */
    [[nodiscard]] const DistCondQParameters &ZCondQ(DistCondQParameters &&par);
    /* <par>.m_Result[i] = P(blk.degree == i) */
    [[nodiscard]] const DistCondQParameters &DistCondQ(DistCondQParameters &&par);
    /* <par>.m_Result[i] = P(blk.degree == i), and compute 3 auxiliary metrics */
    [[nodiscard]] const DistCondQParameters &UCondQ(DistCondQParameters &&par);
    void ClearDistCondQCache();
};

/* Distribution of the degree of a block
 *
 * Note: This class only stores data; computation happens in class Solver
 */
class
    DistCondQParameters
{
public:
    DistCondQParameters(DistCondQParameters &&other) noexcept;
    DistCondQParameters(Block set2ID, int length);

    // [i] = num of shared blocks b/w the block's neighbor and m_BlockSets[i]
    std::vector<int> Sets1;
    int Set2ID; // id such that m_BlockSets[<Set2ID>] contains the block
    int Length; // sum of <sets1>

    /* Note 1: as long as Sets1 and Set2ID are same,
     * identical distribution is guaranteed.
     * Note 2: Sets1, Set2ID, Length are all immutable.
     */
    size_t m_Hash;
    size_t Hash();

    /* List of ids that m_BlockSets[<m_Halves[i]>] is split in two halves
     * by the block's neighbor: one half is of size Sets1[<m_Halves[i]>]
     */
    std::vector<int> m_Halves;
    std::vector<std::vector<Solution>> m_Solutions;
    std::vector<double> m_States;

    /* The probability of each individual degree number */
    std::vector<double> m_Result;

    /* These data are only set by UCondQ
     * m_Probability: E(<at-least-one-safe-block>)
     * m_Expectation: E(<number-of-safe-block>)
     * m_UpperBound: E(1 - <minimal-mine-prob-across-the-board>)
     */
    double m_Probability, m_Expectation, m_UpperBound;

    double m_TotalStates;

    friend bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
};

bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
