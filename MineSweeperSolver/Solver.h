#pragma once
#include "stdafx.h"
#include <vector>
#include "OrthogonalList.h"
#include "Solution.h"
#include <set>
#include <map>

enum class BlockStatus
{
    Unknown = -127,
    Mine = -1,
    Blank = -2
};

typedef int Block;
typedef std::vector<Block> BlockSet;

class DistCondParameters;
class DistCondQParameters;

class DLL_API Solver
{
public:
    explicit Solver(int count);

    BlockStatus GetBlockStatus(Block block) const;
    const BlockStatus *GetBlockStatuses() const;
    double GetProbability(Block block) const;
    const double *GetProbabilities() const;
    const BigInteger &GetTotalStates() const;

    void AddRestrain(Block blk, bool isMine);
    void AddRestrain(const BlockSet &set, int mines);
    void Solve(bool withOverlap, bool withProb);

    const std::vector<BigInteger> &DistributionCondQ(const BlockSet &set, Block blk, int &min);
private:
    std::vector<BlockStatus> m_Manager;
    std::vector<BlockSet> m_BlockSets;
    std::vector<int> m_SetIDs;
    OrthogonalList<int> m_Matrix;
    std::vector<Solution> m_Solutions;
    std::vector<double> m_Probability;
    std::set<std::pair<int, int>> m_Pairs;
    BigInteger m_TotalStates;
    std::multimap<unsigned __int64, std::pair<DistCondQParameters, std::vector<BigInteger>>> m_DistCondQCache;

    void ReduceSet(BlockSet &set, int &outMines, int &outBlanks) const;
    void MergeSets();
    bool ReduceRestrains();
    bool SimpleOverlapAll();
    bool SimpleOverlap(int r1, int r2, bool &rowRemoved);
    void EnumerateSolutions(const std::vector<int> &minors, const OrthogonalList<float> &augmentedMatrix);
    void ProcessSolutions();

    void GetIntersectionCounts(const BlockSet &set1, std::vector<int> &sets1, int &mines, int &blanks) const;

    const std::vector<BigInteger> &DistCondQ(const DistCondQParameters &par);
};

class DistCondQParameters
{
public:
    friend class Solver;
private:
    DistCondQParameters(const std::vector<int> &sets1, Block set2ID, int length);

    const std::vector<int> &Sets1;
    int Set2ID;
    int Length;
    unsigned __int64 m_Hash;

    friend bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
};

bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
