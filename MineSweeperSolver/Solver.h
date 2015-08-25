#pragma once
#include "stdafx.h"
#include <vector>
#include "OrthogonalList.h"
#include "Solution.h"
#include <set>

enum DLL_API BlockStatus
{
    Unknown,
    Mine,
    Blank
};

typedef int Block;
typedef std::vector<Block> BlockSet;

class DLL_API Solver
{
public:
    explicit Solver(int count);

    BlockStatus GetBlockStatus(Block block) const;
    double GetProbability(Block block) const;
    const BigInteger &GetTotalStates() const;

    void AddRestrain(const BlockSet &set, int mines);
    void Solve(bool withProb);
private:
    std::vector<BlockStatus> m_Manager;
    std::vector<BlockSet> m_BlockSets;
    OrthogonalList<double> m_Matrix;
    std::vector<Solution> m_Solutions;
    std::vector<double> m_Probability;
    std::set<std::pair<int, int>> m_Pairs;
    BigInteger m_TotalStates;

    static void Overlap(const BlockSet &setA, const BlockSet &setB, BlockSet &ExceptA, BlockSet &ExceptB, BlockSet &Intersection);
    std::vector<int> OverlapBlockSet(const BlockSet &set);
    void ReduceSet(BlockSet &set, int &outMines, int &outBlanks) const;
    bool ReduceRestrains();
    bool SimpleOverlap();
    bool SimpleOverlap(int r1, int r2);
    static std::vector<int> Gauss(OrthogonalList<double> &matrix);
    void EnumerateSolutions(const std::vector<int> &minors, const OrthogonalList<double> &augmentedMatrix);
    void ProcessSolutions();
};
