#pragma once
#include "stdafx.h"
#include <vector>
#include "OrthogonalList.h"
#include <set>
#include <map>

enum class BlockStatus
{
    Unknown = -127,
    Mine = -1,
    Blank = -2
};

enum class SolvingState
{
    Stale = 0x0,
    Reduce = 0x1,
    Overlap = 0x2,
    Probability = 0x4,
    ZeroProb = 0x8,
    Drained = 0x8000,
    CanOpenForSure = 0x10000
};

inline SolvingState operator&(SolvingState lhs, SolvingState rhs)
{
    return static_cast<SolvingState>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

inline SolvingState operator|(SolvingState lhs, SolvingState rhs)
{
    return static_cast<SolvingState>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline SolvingState operator&=(SolvingState &lhs, SolvingState rhs)
{
    return lhs = (lhs & rhs);
}

inline SolvingState operator|=(SolvingState &lhs, SolvingState rhs)
{
    return lhs = (lhs | rhs);
}

typedef int Block;
typedef std::vector<Block> BlockSet;

class GameMgr;
class Solution;
class DistCondQParameters;

class
    DLL_API Solver
{
public:
    explicit Solver(int count);
    Solver(const Solver &other);
    ~Solver();

    SolvingState GetSolvingState() const;
    BlockStatus GetBlockStatus(Block block) const;
    const BlockStatus *GetBlockStatuses() const;
    double GetProbability(Block block) const;
    const double *GetProbabilities() const;
    double GetTotalStates() const;

    void AddRestrain(Block blk, bool isMine);
    void AddRestrain(const BlockSet &set, int mines);
    void Solve(SolvingState maxDepth, bool shortcut);

    double ZeroCondQ(const BlockSet &set, Block blk);
    double ZerosCondQ(const BlockSet &set, Block blk);
    const std::vector<double> &DistributionCondQ(const BlockSet &set, Block blk, int &min);
    double QuantityCondQ(const BlockSet &set, Block blk);

    friend class Drainer;
    friend double Probe(const GameMgr& mgr, Block blk);
private:
    SolvingState m_State;
    std::vector<BlockStatus> m_Manager;
    std::vector<BlockSet> m_BlockSets;
    std::vector<int> m_SetIDs;
    OrthogonalList<int> m_Matrix;
    std::vector<int> m_Minors;
    std::vector<Solution> m_Solutions;
    std::vector<double> m_Probability;
    double m_TotalStates;
    std::multimap<size_t, DistCondQParameters *> m_DistCondQCache;

    BlockSet m_Reduce_Temp;
    std::vector<int> m_IntersectionCounts_Temp;
    std::set<std::pair<int, int>> m_Pairs_Temp;
    std::vector<int> m_OverlapIndexes_Temp;
    std::vector<int> m_OverlapA_Temp, m_OverlapB_Temp, m_OverlapC_Temp;
    std::vector<std::pair<int, double>> m_GaussVec_Temp;
    std::vector<int> m_Majors_Temp, m_Counts_Temp, m_Stack_Temp, m_Dist_Temp;
    std::vector<double> m_Sums_Temp;
    std::vector<double> m_Exp_Temp;
    std::vector<double> m_DicT_Temp, m_Cases_Temp;
    std::vector<double> m_Add_Temp;

    void MergeSets();
    void ReduceRestrains();
    void SimpleOverlapAll();
    bool SimpleOverlap(int r1, int r2);
    void Gauss(OrthogonalList<double> &matrix);
    void EnumerateSolutions(const OrthogonalList<double> &augmentedMatrix);
    void ProcessSolutions();

    static void Merge(const std::vector<double> &from, std::vector<double> &to);
    void Add(std::vector<double> &from, const std::vector<double> &cases);
    void GetIntersectionCounts(const BlockSet &set1, std::vector<int> &sets1, int &mines) const;

    const DistCondQParameters &ZCondQ(DistCondQParameters &&par);
    const DistCondQParameters &ZsCondQ(DistCondQParameters &&par);
    const DistCondQParameters &DistCondQ(DistCondQParameters &&par);
    void ClearDistCondQCache();
};

class DistCondQParameters
{
public:
    friend class Solver;
private:
    DistCondQParameters(DistCondQParameters &&other);
    DistCondQParameters(Block set2ID, int length);

    std::vector<int> Sets1;
    int Set2ID;
    int Length;
    size_t m_Hash;

    size_t Hash();

    std::vector<double> m_Result;
    double m_Expectation;
    double m_TotalStates;

    friend bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
};

bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);

class Solution
{
public:
    friend class Solver;
    friend class Drainer;
    friend double Probe(const GameMgr& mgr, Block blk);
private:
    std::vector<int> Dist;
    double States;
    double Ratio;
};
