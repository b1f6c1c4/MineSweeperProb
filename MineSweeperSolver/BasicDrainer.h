#pragma once
#include "stdafx.h"
#include <vector>
#include <set>
#include "Solver.h"

#define USE_BASIC_SOLVER

class DLL_API Drainer;

typedef std::vector<BlockStatus> MicroSituation;

class MacroSituation
{
public:
    MacroSituation();
    ~MacroSituation();

    int m_ToOpen;
#ifdef USE_BASIC_SOLVER
    BasicSolver *m_Solver;
#else
    Solver *m_Solver;
#endif

    std::vector<int> m_Degrees;
    double m_BestProb;
    BlockSet m_BestBlocks;
    std::vector<double> m_Probs;
    size_t Hash();

    friend bool operator==(const MacroSituation &lhs, const MacroSituation &rhs);
    friend bool operator!=(const MacroSituation &lhs, const MacroSituation &rhs);

    friend class BasicDrainer;
private:
    std::set<MicroSituation *> m_Micros;
    std::set<MacroSituation *> m_Incomings;
    std::map<Block, std::map<MicroSituation *, MacroSituation *>> m_Transfer;

    size_t m_Hash;

    MacroSituation(const MacroSituation &other);
};
bool operator==(const MacroSituation &lhs, const MacroSituation &rhs);
bool operator!=(const MacroSituation &lhs, const MacroSituation &rhs);

class
    DLL_API BasicDrainer
{
public:
    virtual ~BasicDrainer();

    double GetBestProb() const;
protected:
    BasicDrainer();
    std::vector<BlockSet> m_BlocksR;

    MacroSituation *m_RootMacro;

    void GenerateMicros(const std::vector<BlockSet> &sets, size_t totalStates, const std::vector<Solution> &solutions);
    void Drain();
#ifdef USE_BASIC_SOLVER
    void GenerateRoot(BasicSolver *solver, int toOpen);
#else
    void GenerateRoot(Solver *solver, int toOpen);
#endif
    virtual void HeuristicPruning(MacroSituation *macro, BlockSet &bests) = 0;

    void Update(MacroSituation *&macro);
private:
    std::vector<MicroSituation> m_Micros;
    std::multimap<size_t, MacroSituation *> m_Macros;

    MacroSituation *m_SucceedMacro, *m_FailMacro;

    MacroSituation *GetOrAddMacroSituation(MacroSituation *&macro);

    void SolveMicro(MicroSituation &micro, MacroSituation *macro);
    MacroSituation *SolveMicro(MicroSituation &micro, MacroSituation *macroOld, Block blk);
    void OpenBlock(MicroSituation &micro, MacroSituation *macro, Block blk);
};
