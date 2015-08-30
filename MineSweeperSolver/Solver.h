#pragma once
#include "stdafx.h"
#include <vector>
#include "OrthogonalList.h"
#include <set>
#include <map>
#include "BigInteger.h"

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

class Solution;
class DistCondQParameters;

class
    DLL_API Solver
{
public:
    explicit Solver(int count);
    ~Solver();

    SolvingState GetSolvingState() const;
    BlockStatus GetBlockStatus(Block block) const;
    const BlockStatus *GetBlockStatuses() const;
    double GetProbability(Block block) const;
    const double *GetProbabilities() const;
    const BigInteger &GetTotalStates() const;

    void AddRestrain(Block blk, bool isMine);
    void AddRestrain(const BlockSet &set, int mines);
    void Solve(SolvingState maxDepth, bool shortcut);

    const BigInteger &ZeroCondQ(const BlockSet &set, Block blk);
    const std::vector<BigInteger> &DistributionCondQ(const BlockSet &set, Block blk, int &min);

    friend class Drainer;
private:
    SolvingState m_State;
    std::vector<BlockStatus> m_Manager;
    std::vector<BlockSet> m_BlockSets;
    std::vector<int> m_SetIDs;
    OrthogonalList<int> m_Matrix;
    std::vector<Solution> m_Solutions;
    std::vector<double> m_Probability;
    std::set<std::pair<int, int>> m_Pairs;
    BigInteger m_TotalStates;
    std::multimap<unsigned __int64, DistCondQParameters *> m_DistCondQCache;

    void MergeSets();
    void ReduceRestrains();
    void SimpleOverlapAll();
    bool SimpleOverlap(int r1, int r2);
    void EnumerateSolutions(const std::vector<int> &minors, const OrthogonalList<double> &augmentedMatrix);
    void ProcessSolutions();

    void GetIntersectionCounts(const BlockSet &set1, std::vector<int> &sets1, int &mines) const;

    const BigInteger &ZCondQ(DistCondQParameters &&par);
    const std::vector<BigInteger> &DistCondQ(DistCondQParameters &&par);
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
    unsigned __int64 m_Hash;

    unsigned __int64 Hash();

    std::vector<BigInteger> m_Result;

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
    explicit Solution(std::vector<int> &&dist);
    ~Solution();

    friend class Solver;
    friend class Drainer;
private:
    std::vector<int> Dist;
    BigInteger States;
    double Ratio;
};
