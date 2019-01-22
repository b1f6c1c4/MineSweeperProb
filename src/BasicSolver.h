#pragma once
#include "stdafx.h"
#include <vector>

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

struct Solution;

class
    BasicSolver
{
public:
    explicit BasicSolver(size_t count);
    BasicSolver(size_t count, int mines);
    BasicSolver(const BasicSolver &other);
    virtual ~BasicSolver();

    int CanOpenForSure;

    BlockStatus GetBlockStatus(Block block) const;
    const BlockStatus *GetBlockStatuses() const;
    double GetProbability(Block block) const;
    const double *GetProbabilities() const;
    double GetTotalStates() const;
    const std::vector<BlockSet> &GetBlockSets() const;
    const std::vector<Solution> &GetSolutions() const;

    void AddRestrain(Block blk, bool isMine);
    void AddRestrain(const BlockSet &set, int mines);
    virtual bool Solve(SolvingState maxDepth, bool shortcut);

    friend class Drainer;
protected:
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

    void GetIntersectionCounts(const BlockSet &set1, std::vector<int> &sets1, int &mines) const;
private:
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

    int m_RestMines;

    void DropColumn(int col);
    void DropRow(int row);
    bool ReduceBlockSet(int col);
    bool ReduceRestrainBlank(int row);
    bool ReduceRestrainMine(int row);
    void InversedReduceRestrainMine(int row);

    void MergeSets();
    void ReduceRestrains();
    void SimpleOverlapAll();
    bool SimpleOverlap(int r1, int r2);
    void Gauss(double *matrix, size_t width, size_t height);
    void EnumerateSolutions(const double *matrix, size_t width, size_t height);
    void ProcessSolutions();

#ifdef _DEBUG
    void CheckForConsistency(bool complete);
#endif
};

struct
    Solution
{
    std::vector<int> Dist;
    double States;
    double Ratio;
};
