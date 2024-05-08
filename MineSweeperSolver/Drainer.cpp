#include "Drainer.h"

Drainer::Drainer(const GameMgr &mgr) : m_Mgr(mgr)
{
    for (auto i = 0; i < m_Mgr.m_Blocks.size(); ++i)
    {
        if (m_Mgr.m_Blocks[i].IsOpen || m_Mgr.m_Solver->GetBlockStatus(i) != BlockStatus::Unknown)
            continue;
        m_BlocksLookup.insert(std::make_pair(i, m_Blocks.size()));
        m_Blocks.push_back(i);
    }
    m_BlocksR.resize(m_Blocks.size());
    m_DMines.resize(m_Blocks.size(), 0);
    for (auto i = 0; i < m_Blocks.size(); ++i)
    {
        for (auto blk : m_Mgr.m_BlocksR[m_Blocks[i]])
            switch (m_Mgr.m_Solver->GetBlockStatus(blk))
            {
            case BlockStatus::Unknown:
                m_BlocksR[i].push_back(m_BlocksLookup[blk]);
                break;
            case BlockStatus::Mine:
                ++m_DMines[i];
                break;
            case BlockStatus::Blank:
                break;
            default:
                ASSERT(false);
                break;
            }
    }

#ifdef USE_BASIC_SOLVER
    auto solver = new BasicSolver(m_Blocks.size());
#else
    auto solver = new Solver(m_Blocks.size());
#endif

#ifndef NDEBUG
    solver->m_SetIDs.clear();
    solver->m_SetIDs.resize(m_Blocks.size(), -1);
#endif
    solver->m_BlockSets.clear();
    solver->m_BlockSets.reserve(m_Mgr.m_Solver->m_BlockSets.size());
    for (auto &set : m_Mgr.m_Solver->m_BlockSets)
    {
        solver->m_BlockSets.emplace_back();
        auto &setC = solver->m_BlockSets.back();
        setC.reserve(set.size());
        for (auto blk : set)
        {
            auto blkC = m_BlocksLookup[blk];
#ifndef NDEBUG
            if (blkC == 0)
                ASSERT(m_Blocks[0] == blk);
#endif
            setC.push_back(blkC);
            ASSERT(solver->m_SetIDs[blkC] == -1);
            solver->m_SetIDs[blkC] = solver->m_BlockSets.size() - 1;
            ASSERT(&setC == &solver->m_BlockSets[solver->m_SetIDs[blkC]]);
        }
    }
    solver->m_Matrix = m_Mgr.m_Solver->m_Matrix;
    solver->m_MatrixAugment = m_Mgr.m_Solver->m_MatrixAugment;

    GenerateMicros(solver->m_BlockSets, m_Mgr.m_Solver->m_TotalStates, m_Mgr.m_Solver->m_Solutions);
    GenerateRoot(solver, m_Mgr.m_ToOpen);
}

BlockSet Drainer::GetBestBlocks() const
{
    BlockSet set;
    set.reserve(m_RootMacro->m_BestBlocks.size());
    for (auto blk : m_RootMacro->m_BestBlocks)
        set.push_back(m_Blocks[blk]);
    return set;
}

const double *Drainer::GetBestProbabilities() const
{
    return &*m_Prob.begin();
}

const std::vector<double> &Drainer::GetBestProbabilityList() const
{
    return m_Prob;
}

void Drainer::Update()
{
    auto macro = new MacroSituation();
    macro->m_Degrees = m_RootMacro->m_Degrees;
    for (auto i = 0; i < m_Blocks.size(); ++i)
        if (m_Mgr.m_Blocks[m_Blocks[i]].IsOpen)
            macro->m_Degrees[i] = m_Mgr.m_Blocks[m_Blocks[i]].Degree - m_DMines[i];
    macro->Hash();
    BasicDrainer::Update(macro);

    if (!m_RootMacro->m_Probs.empty())
    {
        m_Prob.clear();
        m_Prob.resize(m_Mgr.m_Blocks.size(), -1);
        for (auto i = 0; i < m_Blocks.size(); ++i)
            m_Prob[m_Blocks[i]] = m_RootMacro->m_Probs[i];
    }
    for (auto i = 0; i < m_Mgr.m_Blocks.size(); ++i)
        switch (m_Mgr.m_Solver->GetBlockStatus(i))
        {
        case BlockStatus::Unknown:
            ASSERT(m_Prob[i] >= 0 && m_Prob[i] <= 1);
            break;
        case BlockStatus::Mine:
            ASSERT(m_Prob[i] == -1);
            m_Prob[i] = 0;
            break;
        case BlockStatus::Blank:
            ASSERT(m_Prob[i] == -1);
            m_Prob[i] = 1;
            break;
        default:
            ASSERT(false);
        }
}

int Drainer::FrontierDist(const MacroSituation *macro, Block blk) const
{
    auto &bt = m_Mgr.m_Blocks[blk];
    auto d = MAX(m_Mgr.m_TotalWidth, m_Mgr.m_TotalHeight);
    for (auto b : m_Mgr.m_Blocks)
    {
        if (macro->m_Degrees[b.Index] < 0)
            continue;
        auto v = MAX(abs(b.X - bt.X), abs(b.Y - bt.Y));
        if (v < d)
            d = v;
    }
    return d;
}

void Drainer::HeuristicPruning(MacroSituation *macro, BlockSet &bests)
{
    if (!m_Mgr.BasicStrategy.PruningEnabled)
        return;
#define LARGEST(exp) Largest(bests, [macro](Block blk) -> double { return exp; })
    if (macro->m_Solver->GetTotalStates() <= m_Mgr.BasicStrategy.ExhaustCriterion)
        return;
    for (auto heu : m_Mgr.BasicStrategy.PruningDecisionTree)
        switch (heu)
        {
        case HeuristicMethod::MinMineProb:
            LARGEST(-macro->m_Solver->GetProbability(blk));
            break;
#ifdef USE_BASIC_SOLVER
        case HeuristicMethod::MaxZeroProb:
        case HeuristicMethod::MaxZerosProb:
        case HeuristicMethod::MaxZerosExp:
        case HeuristicMethod::MaxQuantityExp:
        case HeuristicMethod::MinFrontierDist:
        default:
            break;
#else
        case HeuristicMethod::MaxZeroProb:
            LARGEST(macro->m_Solver->ZeroCondQ(m_BlocksR[blk], blk));
            break;
        case HeuristicMethod::MaxZerosProb:
            LARGEST(macro->m_Solver->ZerosCondQ(m_BlocksR[blk], blk));
            break;
        case HeuristicMethod::MaxZerosExp:
            LARGEST(macro->m_Solver->ZerosECondQ(m_BlocksR[blk], blk));
            break;
        case HeuristicMethod::MaxQuantityExp:
            LARGEST(macro->m_Solver->QuantityCondQ(m_BlocksR[blk], blk));
            break;
        case HeuristicMethod::MinFrontierDist:
            LARGEST(-FrontierDist(macro, blk));
            break;
        default:
            break;
#endif
        }
}
