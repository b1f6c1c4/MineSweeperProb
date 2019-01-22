#pragma once
#include "stdafx.h"
#include "BasicSolver.h"
#include <map>
#include <functional>

class DistCondQParameters;

class
    Solver : public BasicSolver
{
public:
    explicit Solver(size_t count);
    Solver(size_t count, int mines);
    Solver(const Solver &other);
    ~Solver();

    bool Solve(SolvingState maxDepth, bool shortcut) override;

    const DistCondQParameters &GetDistInfo(const BlockSet &set, Block blk, int &min);

    double ZeroCondQ(const BlockSet &set, Block blk);
    double ZerosCondQ(const BlockSet &set, Block blk);
    double ZerosECondQ(const BlockSet &set, Block blk);
    double UpperBoundCondQ(const BlockSet &set, Block blk);
    const std::vector<double> &DistributionCondQ(const BlockSet &set, Block blk, int &min);
    double QuantityCondQ(const BlockSet &set, Block blk);

    friend class Drainer;
private:
    std::multimap<size_t, DistCondQParameters *> m_DistCondQCache;

    std::vector<double> m_DicT_Temp, m_Cases_Temp;
    std::vector<double> m_Add_Temp;


    static void Merge(const std::vector<double> &from, std::vector<double> &to);
    void Add(std::vector<double> &from, const std::vector<double> &cases);

    DistCondQParameters PackParameters(const BlockSet &set, Block blk, int &min) const;

    void GetHalves(DistCondQParameters &par) const;
    void EnumerateSolutions(DistCondQParameters &par) const;

    DistCondQParameters *TryGetCache(DistCondQParameters &&par, std::function<bool(const DistCondQParameters &)> pre);
    const DistCondQParameters &ZCondQ(DistCondQParameters &&par);
    const DistCondQParameters &DistCondQ(DistCondQParameters &&par);
    const DistCondQParameters &UCondQ(DistCondQParameters &&par);
    void ClearDistCondQCache();
};

class
    DistCondQParameters
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

    friend bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
    friend bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
};

bool operator==(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
bool operator!=(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
bool operator<(const DistCondQParameters &lhs, const DistCondQParameters &rhs);
