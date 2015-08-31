#pragma once
#include <vector>
#include "Solver.h"
#include "GameMgr.h"

class GameMgr;
class Drainer;

typedef std::vector<BlockStatus> MicroSituation;

class MacroSituation
{
public:
    ~MacroSituation();

    friend bool operator==(const MacroSituation &lhs, const MacroSituation &rhs);
    friend bool operator!=(const MacroSituation &lhs, const MacroSituation &rhs);

    friend class Drainer;
private:
    std::vector<int> m_Degrees;
    int m_ToOpen;
    Solver *m_Solver;

    std::set<MicroSituation *> m_Micros;
    std::set<MacroSituation *> m_Incomings;
    std::map<Block, std::map<MicroSituation *, MacroSituation *>> m_Transfer;

    double m_BestProb;
    BlockSet m_BestBlocks;
    std::vector<double> m_Probs;

    size_t m_Hash;

    MacroSituation();
    MacroSituation(const MacroSituation &other);

    size_t Hash();
};

bool operator==(const MacroSituation &lhs, const MacroSituation &rhs);
bool operator!=(const MacroSituation &lhs, const MacroSituation &rhs);

class Drainer
{
public:
    explicit Drainer(const GameMgr &mgr);
    ~Drainer();

    double GetBestProb() const;
    BlockSet GetBestBlocks() const;
    const double *GetBestProbabilities() const;

    void Update();
private:
    const GameMgr &m_Mgr;
    std::vector<MicroSituation> m_Micros;
    std::multimap<size_t, MacroSituation *> m_Macros;
    std::vector<Block> m_Blocks;
    std::map<Block, Block> m_BlocksLookup;
    std::vector<BlockSet> m_BlocksR;
    std::vector<int> m_DMines;
    std::vector<double> m_Prob;

    MacroSituation *m_RootMacro;
    MacroSituation *m_SucceedMacro, *m_FailMacro;

    MacroSituation *GetOrAddMacroSituation(MacroSituation *&macro);

    void GenerateMicros();
    void SolveMicro(MicroSituation &micro, MacroSituation *macro);
    MacroSituation *SolveMicro(MicroSituation &micro, MacroSituation *macroOld, Block blk);
    void OpenBlock(MicroSituation &micro, MacroSituation *macro, Block blk);
};
