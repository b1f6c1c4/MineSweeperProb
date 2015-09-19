#pragma once
#include "stdafx.h"
#include <vector>
#include <map>
#include <functional>

enum class DLL_API BlockStatus
{
    Unknown = -127,
    Mine = -1,
    Blank = -2
};

enum class DLL_API SolvingState
{
    Stale = 0x0,
    Reduce = 0x1,
    Overlap = 0x2,
    Probability = 0x4,
    Heuristic = 0x8,
    Drained = 0x8000
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
typedef size_t Container;

#define CONT_ZERO static_cast<Container>(0)
#define CONT_ONE static_cast<Container>(1)
#define CONT_SIZE (sizeof(Container) * 8)
#define CONTS(offset) (((offset) + CONT_SIZE - 1) / CONT_SIZE)
#define CNT(offset) ((offset) / CONT_SIZE)
#define SHF(offset) ((offset) % CONT_SIZE)
#define MASK(shift) (CONT_ONE << (shift))
#define MASKL(lng) (MASK((lng)) - CONT_ONE)
#define MASKH(lng) (~(MASK(CONT_SIZE - (lng)) - CONT_ONE))
#define LB1(val) (MASK(0) & (val))
#define HB1(val) ((MASK(CONT_SIZE - 1) & (val)) >> (CONT_SIZE - 1))
#define LB(val, lng) (MASKL((lng)) & (val))
#define HB(val, lng) ((MASKH((lng)) & (val)) >> (CONT_SIZE - (lng)))
#define B(val, shift) ((MASK((shift)) & (val)) >> (shift))
#define Z(val, shift) (B((val), (shift)) == CONT_ZERO)
#define NZ(val, shift) (B((val), (shift)) != CONT_ZERO)
#define SB(lval, shift) (lval) |= MASK((shift))
#define CB(lval, shift) (lval) &= ~MASK((shift))

class DLL_API GameMgr;
class DLL_API Solution;
class DLL_API DistCondQParameters;

class
    DLL_API Solver
{
public:
    explicit Solver(size_t count);
    Solver(size_t count, int mines);
    Solver(const Solver &other);
    ~Solver();

    int CanOpenForSure;

    BlockStatus GetBlockStatus(Block block) const;
    const BlockStatus *GetBlockStatuses() const;
    double GetProbability(Block block) const;
    const double *GetProbabilities() const;
    double GetTotalStates() const;
    const DistCondQParameters &GetDistInfo(const BlockSet &set, Block blk, int &min);

    void AddRestrain(Block blk, bool isMine);
    void AddRestrain(const BlockSet &set, int mines);
    void Solve(SolvingState maxDepth, bool shortcut);

    double ZeroCondQ(const BlockSet &set, Block blk);
    double ZerosCondQ(const BlockSet &set, Block blk);
    double ZerosECondQ(const BlockSet &set, Block blk);
    double UpperBoundCondQ(const BlockSet &set, Block blk);
    const std::vector<double> &DistributionCondQ(const BlockSet &set, Block blk, int &min);
    double QuantityCondQ(const BlockSet &set, Block blk);

    friend class Drainer;
private:
    SolvingState m_State;
    std::vector<BlockStatus> m_Manager;
    std::vector<BlockSet> m_BlockSets;
    std::vector<int> m_SetIDs;
    std::vector<std::vector<Container>> m_Matrix;
    std::vector<int> m_MatrixAugment;
    std::vector<int> m_Minors;
    std::vector<Solution> m_Solutions;
    std::vector<double> m_Probability;
    double m_TotalStates;
    std::multimap<size_t, DistCondQParameters *> m_DistCondQCache;

    BlockSet m_Reduce_Temp;
    std::vector<size_t> m_ReduceCount_Temp;
    std::vector<int> m_IntersectionCounts_Temp;
    bool *m_Pairs_Temp;
    size_t m_Pairs_Temp_Size;
    std::vector<int> m_OverlapIndexes_Temp;
    std::vector<Container> m_OverlapA_Temp, m_OverlapB_Temp, m_OverlapC_Temp;
    std::vector<double> m_GaussVec_Temp;
    std::vector<std::vector<int>> m_NonZero_Temp;
    std::vector<size_t> m_Counts_Temp;
    std::vector<int> m_Majors_Temp, m_Stack_Temp, m_Dist_Temp;
    std::vector<double> m_Sums_Temp;
    std::vector<double> m_Exp_Temp;
    std::vector<double> m_DicT_Temp, m_Cases_Temp;
    std::vector<double> m_Add_Temp;

    void DropColumn(int col);
    void DropRow(int row);
    bool ReduceBlockSet(int col);
    bool ReduceRestrainBlank(int row);
    bool ReduceRestrainMine(int row);

    void MergeSets();
    void ReduceRestrains();
    void SimpleOverlapAll();
    bool SimpleOverlap(int r1, int r2);
    void Gauss(double *matrix, size_t width, size_t height);
    void EnumerateSolutions(const double *matrix, size_t width, size_t height);
    void ProcessSolutions();

    static void Merge(const std::vector<double> &from, std::vector<double> &to);
    void Add(std::vector<double> &from, const std::vector<double> &cases);
    void GetIntersectionCounts(const BlockSet &set1, std::vector<int> &sets1, int &mines) const;

    DistCondQParameters PackParameters(const BlockSet &set, Block blk, int &min) const;

    void GetHalves(DistCondQParameters &par) const;
    void GetSolutions(DistCondQParameters &par) const;

    DistCondQParameters *TryGetCache(DistCondQParameters &&par, std::function<bool(const DistCondQParameters &)> pre);
    const DistCondQParameters &ZCondQ(DistCondQParameters &&par);
    const DistCondQParameters &DistCondQ(DistCondQParameters &&par);
    const DistCondQParameters &UCondQ(DistCondQParameters &&par);
    void ClearDistCondQCache();

#ifdef _DEBUG
    void CheckForConsistency(bool complete);
#endif
};

class
    DLL_API DistCondQParameters
{
public:
    DistCondQParameters(DistCondQParameters &&other);
    DistCondQParameters(Block set2ID, int length);

    std::vector<int> Sets1;
    int Set2ID;
    int Length;
    size_t m_Hash;

    size_t Hash();

    std::vector<int> m_Halves;
    std::vector<std::vector<Solution>> m_Solutions;
    std::vector<double> m_States;

    std::vector<double> m_Result;
    double m_Probability, m_Expectation, m_UpperBound;
    double m_TotalStates;

    friend DLL_API bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend DLL_API bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend DLL_API bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
};

DLL_API bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
DLL_API bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
DLL_API bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);

class
    DLL_API Solution
{
public:
    friend class Solver;
    friend class Drainer;
private:
    std::vector<int> Dist;
    double States;
    double Ratio;
};
